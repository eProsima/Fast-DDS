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
 * This file contains negative tests related to the TypeObjectUtils API.
 */

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include <fastdds/xtypes/type_representation/BuiltinAnnotations.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

// Build TypeObjectHashId object with wrong discriminator.
TEST(TypeObjectUtilsTests, build_type_object_hash_id_wrong_discriminator)
{
    uint8_t bad_discriminator = EK_BOTH;
    EquivalenceHash hash;
    EXPECT_THROW(TypeObjectHashId type_object_hash_id = TypeObjectUtils::build_type_object_hash_id(
        bad_discriminator, hash), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectHashId type_object_hash_id = TypeObjectUtils::build_type_object_hash_id(
        EK_MINIMAL, hash));
    EXPECT_NO_THROW(TypeObjectHashId type_object_hash_id = TypeObjectUtils::build_type_object_hash_id(
        EK_COMPLETE, hash));
}

// Build StringSTypeDefn with bound equal 0 (INVALID_SBOUND).
TEST(TypeObjectUtilsTests, build_string_s_type_defn_invalid_bound)
{
    SBound wrong_bound = 0;
    EXPECT_THROW(StringSTypeDefn string_l_type_defn = TypeObjectUtils::build_string_s_type_defn(wrong_bound),
        InvalidArgumentError);
    EXPECT_NO_THROW(StringSTypeDefn string_l_type_defn = TypeObjectUtils::build_string_s_type_defn(1));
}


// Build StringLTypeDefn with bound smaller than 256.
TEST(TypeObjectUtilsTests, build_string_l_type_defn_small_bound)
{
    LBound wrong_bound = 255;
    EXPECT_THROW(StringLTypeDefn string_l_type_defn = TypeObjectUtils::build_string_l_type_defn(wrong_bound),
        InvalidArgumentError);
    EXPECT_NO_THROW(StringLTypeDefn string_l_type_defn = TypeObjectUtils::build_string_l_type_defn(256));
}

#if !defined(NDEBUG)
// Build PlainCollectionHeader with inconsistent CollectionElementFlag.
TEST(TypeObjectUtilsTests, build_plain_collection_header_inconsistent_element_flags)
{
    CollectionElementFlag wrong_element_flag = 0;
    CollectionElementFlag correct_element_flag = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM,
        false);
    EXPECT_THROW(PlainCollectionHeader plain_collection_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, wrong_element_flag), InvalidArgumentError);
    EXPECT_NO_THROW(PlainCollectionHeader plain_collection_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, correct_element_flag));
}
#endif // !defined(NDEBUG)

// Build PlainSequenceSElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_sequence_s_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        wrong_header, 10, test_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        wrong_header, 10, test_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        complete_header, 10, test_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check SBound consistency
    SBound wrong_bound = 0;
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        complete_header, wrong_bound, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        complete_header, 10, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        minimal_header, 10, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        fully_descriptive_header, 10, test_identifier));
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        complete_header, 10, test_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        minimal_header, 10, test_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        fully_descriptive_header, 10, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        complete_header, 10, test_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        minimal_header, 10, test_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        fully_descriptive_header, 10, test_identifier), InvalidArgumentError);
}

// Build PlainSequenceLElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_sequence_l_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        wrong_header, 256, test_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        wrong_header, 256, test_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        complete_header, 256, test_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check LBound consistency
    LBound wrong_bound = 255;
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        complete_header, wrong_bound, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        complete_header, 256, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        minimal_header, 256, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        fully_descriptive_header, 256, test_identifier));
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        complete_header, 256, test_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        minimal_header, 256, test_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        fully_descriptive_header, 256, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        complete_header, 256, test_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        minimal_header, 256, test_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        fully_descriptive_header, 256, test_identifier), InvalidArgumentError);
}

// Build PlainArraySElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_array_s_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    SBoundSeq bound_seq;
    TypeObjectUtils::add_array_dimension(bound_seq, 10);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        wrong_header, bound_seq, test_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        wrong_header, bound_seq, test_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check SBoundSeq consistency
    SBoundSeq wrong_bound_seq;
    // Empty array_bound_seq
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);
    // Zero element
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, 10);
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, 0);
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier));
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, bound_seq, test_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        minimal_header, bound_seq, test_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier), InvalidArgumentError);
}

// Build PlainArrayLElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_array_l_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    LBoundSeq bound_seq;
    TypeObjectUtils::add_array_dimension(bound_seq, 256);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        wrong_header, bound_seq, test_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        wrong_header, bound_seq, test_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check SBoundSeq consistency
    LBoundSeq wrong_bound_seq;
    // Empty array_bound_seq
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);
    // Non-large bound dimension
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, 10);
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier));
    // At least one dimension should be large
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, 256);
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        fully_descriptive_header, wrong_bound_seq, test_identifier));
    // Zero element
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, 0);
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        fully_descriptive_header, wrong_bound_seq, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, bound_seq, test_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        complete_header, bound_seq, test_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        minimal_header, bound_seq, test_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
        fully_descriptive_header, bound_seq, test_identifier), InvalidArgumentError);
}

// Build PlainMapSTypeDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_map_s_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        wrong_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        wrong_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check SBound consistency
    SBound wrong_bound = 0;
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        complete_header, wrong_bound, test_identifier, flags, key_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        minimal_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // Wrong key_flags
    CollectionElementFlag wrong_flags = 0;
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, wrong_flags, key_identifier), InvalidArgumentError);
#if !defined(NDEBUG)
    // Uninitialized key_identifier
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Non-integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_FLOAT32));
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with fully-descriptive header and integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_INT64));
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier));
#if !defined(NDEBUG)
    // Inconsistent string TypeIdentifier
    StringSTypeDefn wrong_string_type_def;
    EXPECT_NO_THROW(key_identifier->string_sdefn(wrong_string_type_def));
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    StringSTypeDefn string_type_def = TypeObjectUtils::build_string_s_type_defn(50);
    EXPECT_NO_THROW(key_identifier->string_sdefn(string_type_def));
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier));
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        complete_header, 10, test_identifier, flags, key_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        minimal_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        minimal_header, 10, test_identifier, flags, key_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainMapSTypeDefn plain_seq = TypeObjectUtils::build_plain_map_s_type_defn(
        fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
}

// Build PlainMapLTypeDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_map_l_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        wrong_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        wrong_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        complete_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check LBound consistency
    LBound wrong_bound = 255;
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        complete_header, wrong_bound, test_identifier, flags, key_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        complete_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        minimal_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue::BOTH, flags);
    // Wrong key_flags
    CollectionElementFlag wrong_flags = 0;
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, wrong_flags, key_identifier), InvalidArgumentError);
#if !defined(NDEBUG)
    // Uninitialized key_identifier
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Non-integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_FLOAT32));
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with fully-descriptive header and integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_INT64));
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier));
#if !defined(NDEBUG)
    // Inconsistent string TypeIdentifier
    StringSTypeDefn wrong_string_type_def;
    EXPECT_NO_THROW(key_identifier->string_sdefn(wrong_string_type_def));
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    StringSTypeDefn string_type_def = TypeObjectUtils::build_string_s_type_defn(50);
    EXPECT_NO_THROW(key_identifier->string_sdefn(string_type_def));
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier));
    // Change discriminator to EK_COMPLETE
    EquivalenceHash hash;
    test_identifier->equivalence_hash(hash);
    // TypeIdentifier consistent with complete header
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        complete_header, 1000, test_identifier, flags, key_identifier));
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        minimal_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    // Change discriminator to EK_MINIMAL
    EXPECT_NO_THROW(test_identifier->_d(EK_MINIMAL));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        complete_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with minimal header
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        minimal_header, 1000, test_identifier, flags, key_identifier));
    // TypeIdentifier inconsistent with fully-descriptive header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
        fully_descriptive_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
}

// Register small string/wstring. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_string)
{
    StringSTypeDefn string_defn = TypeObjectUtils::build_string_s_type_defn(32);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_string_type_identifier(string_defn,
        "small_string"));
    // Registering twice the same TypeIdentifier should not fail
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_string_type_identifier(string_defn,
        "small_string"));
    // Registering another TypeIdentifier with the same name should return RETCODE_BAD_PARAMETER
    StringSTypeDefn another_string_defn = TypeObjectUtils::build_string_s_type_defn(100);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_string_type_identifier(
        another_string_defn, "small_string"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_string_type_identifier(
        another_string_defn, type_name));
}

// Register large string/wstring. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_string)
{
    StringLTypeDefn string_defn = TypeObjectUtils::build_string_l_type_defn(1000);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_string_type_identifier(string_defn,
        "large_string"));
    // Registering twice the same TypeIdentifier should not fail
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_string_type_identifier(string_defn,
        "large_string"));
    // Registering another TypeIdentifier with the same name should return RETCODE_BAD_PARAMETER
    StringLTypeDefn another_string_defn = TypeObjectUtils::build_string_l_type_defn(2000);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_string_type_identifier(
        another_string_defn, "large_string"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_string_type_identifier(
        another_string_defn, type_name));
}

// Register small sequence. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_sequence)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        header, 255, primitive_identifier);
    primitive_identifier->_d(TK_INT16);
    PlainSequenceSElemDefn another_plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        header, 255, primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_sequence_type_identifier(plain_seq,
        "small_sequence"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_sequence_type_identifier(plain_seq,
        "small_sequence"));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_sequence_type_identifier(
        another_plain_seq, "small_sequence"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_sequence_type_identifier(
        another_plain_seq, type_name));
}

// Register large sequence. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_sequence)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        header, 256, primitive_identifier);
    primitive_identifier->_d(TK_INT16);
    PlainSequenceLElemDefn another_plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        header, 256, primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_sequence_type_identifier(plain_seq,
        "large_sequence"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_sequence_type_identifier(plain_seq,
        "large_sequence"));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_sequence_type_identifier(
        another_plain_seq, "large_sequence"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_sequence_type_identifier(
        another_plain_seq, type_name));
}

// Register small array. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_array)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    SBoundSeq array_bounds;
    TypeObjectUtils::add_array_dimension(array_bounds, 26);
    PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(header, array_bounds,
        primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_array_type_identifier(plain_array,
        "small_array"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_array_type_identifier(plain_array,
        "small_array"));
    TypeObjectUtils::add_array_dimension(array_bounds, 100);
    PlainArraySElemDefn another_plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(header, array_bounds,
        primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_array_type_identifier(
        another_plain_array, "small_array"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_array_type_identifier(
        another_plain_array, type_name));
}

// Register large array. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_array)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    LBoundSeq array_bounds;
    TypeObjectUtils::add_array_dimension(array_bounds, 260);
    PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(header, array_bounds,
        primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_array_type_identifier(plain_array,
        "large_array"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_array_type_identifier(plain_array,
        "large_array"));
    TypeObjectUtils::add_array_dimension(array_bounds, 1000);
    PlainArrayLElemDefn another_plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(header, array_bounds,
        primitive_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_array_type_identifier(
        another_plain_array, "large_array"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_array_type_identifier(
        another_plain_array, type_name));
}

// Register small map. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_map)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_UINT32);
    PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(header, 10, primitive_identifier, flags,
        primitive_identifier);
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    key_identifier->_d(TK_INT8);
    PlainMapSTypeDefn another_plain_map = TypeObjectUtils::build_plain_map_s_type_defn(header, 10, primitive_identifier,
        flags, key_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_map_type_identifier(plain_map,
        "small_map"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_s_map_type_identifier(plain_map,
        "small_map"));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_map_type_identifier(
        another_plain_map, "small_map"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_map_type_identifier(
        another_plain_map, type_name));
}

// Register large map. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_map)
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_UINT32);
    PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(header, 500, primitive_identifier, flags,
        primitive_identifier);
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    key_identifier->_d(TK_INT8);
    PlainMapLTypeDefn other_plain_map = TypeObjectUtils::build_plain_map_l_type_defn(header, 500, primitive_identifier,
        flags, key_identifier);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_map_type_identifier(plain_map,
        "large_map"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_l_map_type_identifier(plain_map,
        "large_map"));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_map_type_identifier(
        other_plain_map, "large_map"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_map_type_identifier(
        other_plain_map, type_name));
}

// Build CompleteMemberDetail with empty name
TEST(TypeObjectUtilsTests, build_complete_member_detail_empty_member_name)
{
    EXPECT_THROW(CompleteMemberDetail detail = TypeObjectUtils::build_complete_member_detail("",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteMemberDetail detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>()));
}

// Build CompleteStructMember with inconsistent @hashid annotation value and member id
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_hashid_member_id)
{
    StructMemberFlag basic_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false, false,
        false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonStructMember common = TypeObjectUtils::build_common_struct_member(0x047790DA, basic_flags, type_id);
    AppliedBuiltinMemberAnnotations wrong_ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("color"));
    AppliedBuiltinMemberAnnotations ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("shapesize"));
    AppliedBuiltinMemberAnnotations empty_ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>(""));
    CompleteMemberDetail wrong_detail = TypeObjectUtils::build_complete_member_detail("member_name", wrong_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail detail = TypeObjectUtils::build_complete_member_detail("member_name", ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail empty_detail = TypeObjectUtils::build_complete_member_detail("shapesize", empty_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail other_wrong_detail = TypeObjectUtils::build_complete_member_detail("name", empty_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(common, wrong_detail),
        InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(common, detail));
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(common,
        other_wrong_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(common, empty_detail));
}

/**
 * Auxiliary methods that returns the CompleteMemberDetail with the try construct annotation set with each possible
 * value
 */
void applied_annotation_seq_try_construct(
        AppliedAnnotationSeq& discard_anns,
        AppliedAnnotationSeq& trim_anns,
        AppliedAnnotationSeq& use_default_anns)
{
    AnnotationParameterValue discard_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        TryConstructFailAction::DISCARD);
    AnnotationParameterValue trim_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        TryConstructFailAction::TRIM);
    AnnotationParameterValue use_default_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        TryConstructFailAction::USE_DEFAULT);
    NameHash value_hash = TypeObjectUtils::name_hash("value");
    AppliedAnnotationParameter discard_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        discard_param_value);
    AppliedAnnotationParameter trim_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        trim_param_value);
    AppliedAnnotationParameter use_default_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        use_default_param_value);
    AppliedAnnotationParameterSeq discard_param_seq;
    AppliedAnnotationParameterSeq trim_param_seq;
    AppliedAnnotationParameterSeq use_default_param_seq;
    TypeObjectUtils::add_applied_annotation_parameter(discard_param_seq, discard_param);
    TypeObjectUtils::add_applied_annotation_parameter(trim_param_seq, trim_param);
    TypeObjectUtils::add_applied_annotation_parameter(use_default_param_seq, use_default_param);
    TypeIdentifierPair try_construct_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            try_construct_annotation_name, try_construct_type_id));
    AppliedAnnotation discard_ann = TypeObjectUtils::build_applied_annotation(try_construct_type_id.type_identifier1(),
        discard_param_seq);
    AppliedAnnotation trim_ann = TypeObjectUtils::build_applied_annotation(try_construct_type_id.type_identifier1(),
        trim_param_seq);
    AppliedAnnotation use_default_ann = TypeObjectUtils::build_applied_annotation(
        try_construct_type_id.type_identifier1(), use_default_param_seq);
    TypeObjectUtils::add_applied_annotation(discard_anns, discard_ann);
    TypeObjectUtils::add_applied_annotation(trim_anns, trim_ann);
    TypeObjectUtils::add_applied_annotation(use_default_anns, use_default_ann);
}

void complete_member_detail_try_construct(
        CompleteMemberDetail& discard_detail,
        CompleteMemberDetail& trim_detail,
        CompleteMemberDetail& use_default_detail)
{
    AppliedAnnotationSeq discard_anns;
    AppliedAnnotationSeq trim_anns;
    AppliedAnnotationSeq use_default_anns;
    applied_annotation_seq_try_construct(discard_anns, trim_anns, use_default_anns);
    discard_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(discard_anns));
    trim_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(trim_anns));
    use_default_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(use_default_anns));
}

// Build CompleteStructMember with inconsistent @try_construct annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_try_construct)
{
    StructMemberFlag discard_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false, false,
        false, false);
    StructMemberFlag trim_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::TRIM, false, false, false,
        false);
    StructMemberFlag use_default_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::USE_DEFAULT, false,
        false, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonStructMember discard_common = TypeObjectUtils::build_common_struct_member(0, discard_flags, type_id);
    CommonStructMember trim_common = TypeObjectUtils::build_common_struct_member(0, trim_flags, type_id);
    CommonStructMember use_default_common = TypeObjectUtils::build_common_struct_member(0, use_default_flags, type_id);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(trim_common,
        empty_member_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(use_default_common,
        empty_member_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(discard_common,
        empty_member_detail));
    CompleteMemberDetail discard_detail;
    CompleteMemberDetail trim_detail;
    CompleteMemberDetail use_default_detail;
    complete_member_detail_try_construct(discard_detail, trim_detail, use_default_detail);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(trim_common,
        discard_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(trim_common,
        trim_detail));
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(trim_common,
        use_default_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(use_default_common,
        discard_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(use_default_common,
        trim_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(use_default_common,
        use_default_detail));
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(discard_common,
        discard_detail));
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(discard_common,
        trim_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(discard_common,
        use_default_detail), InvalidArgumentError);
}

/**
 * Auxiliary methods to return the CompleteMemberDetail for a boolean builtin annotation
 */
void applied_annotation_seq_boolean(
        AppliedAnnotationSeq& anns,
        AppliedAnnotationSeq& non_anns,
        const std::string& annotation_name)
{
    AnnotationParameterValue ann_param_value = TypeObjectUtils::build_annotation_parameter_value(true);
    AnnotationParameterValue non_ann_param_value = TypeObjectUtils::build_annotation_parameter_value(false);
    NameHash value_hash = TypeObjectUtils::name_hash("value");
    AppliedAnnotationParameter ann_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        ann_param_value);
    AppliedAnnotationParameter non_ann_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        non_ann_param_value);
    AppliedAnnotationParameterSeq ann_param_seq;
    AppliedAnnotationParameterSeq non_ann_param_seq;
    TypeObjectUtils::add_applied_annotation_parameter(ann_param_seq, ann_param);
    TypeObjectUtils::add_applied_annotation_parameter(non_ann_param_seq, non_ann_param);
    TypeIdentifierPair ann_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            annotation_name, ann_type_id));
    AppliedAnnotation ann = TypeObjectUtils::build_applied_annotation(ann_type_id.type_identifier1(),
        ann_param_seq);
    AppliedAnnotation non_ann = TypeObjectUtils::build_applied_annotation(ann_type_id.type_identifier1(),
        non_ann_param_seq);
    TypeObjectUtils::add_applied_annotation(anns, ann);
    TypeObjectUtils::add_applied_annotation(non_anns, non_ann);
}

void complete_member_detail_boolean(
        CompleteMemberDetail& detail,
        CompleteMemberDetail& non_detail,
        const std::string& annotation_name)
{
    AppliedAnnotationSeq anns;
    AppliedAnnotationSeq non_anns;
    applied_annotation_seq_boolean(anns, non_anns, annotation_name);
    detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(anns));
    non_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(non_anns));
}

/**
 * Auxiliary method to run boolean type builtin annotations in structures
 */
void boolean_builtin_annotations_check(
        StructMemberFlag flags,
        const std::string& annotation_name)
{
    StructMemberFlag basic_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false, false,
        false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonStructMember basic_common = TypeObjectUtils::build_common_struct_member(0, basic_flags, type_id);
    CommonStructMember ann_common = TypeObjectUtils::build_common_struct_member(0, flags, type_id);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(ann_common,
        empty_member_detail), InvalidArgumentError);
    CompleteMemberDetail detail;
    CompleteMemberDetail non_detail;
    complete_member_detail_boolean(detail, non_detail, annotation_name);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(ann_common,
        non_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(basic_common,
        detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(basic_common,
        non_detail));
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(ann_common,
        detail));
}

// Build CompleteStructMember with inconsistent @optional annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_optional)
{
    StructMemberFlag optional_flag = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, true, false,
        false, false);
    boolean_builtin_annotations_check(optional_flag, optional_annotation_name);
}

// Build CompleteStructMember with inconsistent @must_understand annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_must_understand)
{
    StructMemberFlag must_understand_flag = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false,
        true, false, false);
    boolean_builtin_annotations_check(must_understand_flag, must_understand_annotation_name);
}

// Build CompleteStructMember with inconsistent @key annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_key)
{
    StructMemberFlag key_flag = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false,
        false, true, false);
    boolean_builtin_annotations_check(key_flag, key_annotation_name);
}

// Build CompleteStructMember with inconsistent @external annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_external)
{
    StructMemberFlag external_flag = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false,
        false, false, true);
    boolean_builtin_annotations_check(external_flag, external_annotation_name);
}

/**
 * Auxiliary method to return a CompleteMemberDetail with @id annotation set
 */
void complete_member_detail_id(
        CompleteMemberDetail& id_detail)
{
    AnnotationParameterValue id_param_value = TypeObjectUtils::build_annotation_parameter_value(
        static_cast<uint32_t>(101));
    NameHash value_hash = TypeObjectUtils::name_hash("value");
    AppliedAnnotationParameter id_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        id_param_value);
    AppliedAnnotationParameterSeq id_param_seq;
    TypeObjectUtils::add_applied_annotation_parameter(id_param_seq, id_param);
    TypeIdentifierPair id_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            id_annotation_name, id_type_id));
    AppliedAnnotation id_ann = TypeObjectUtils::build_applied_annotation(id_type_id.type_identifier1(),
        id_param_seq);
    AppliedAnnotationSeq id_anns;
    TypeObjectUtils::add_applied_annotation(id_anns, id_ann);
    id_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(id_anns));
}

// Build CompleteStructMember with inconsistent @id annotation
TEST(TypeObjectUtilsTests, build_complete_struct_member_inconsistent_id)
{
    StructMemberFlag basic_flags = TypeObjectUtils::build_struct_member_flag(TryConstructKind::DISCARD, false, false,
        false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonStructMember basic_common = TypeObjectUtils::build_common_struct_member(0, basic_flags, type_id);
    CommonStructMember id_common = TypeObjectUtils::build_common_struct_member(101, basic_flags, type_id);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(id_common,
        empty_member_detail));
    CompleteMemberDetail id_detail;
    complete_member_detail_id(id_detail);
    EXPECT_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(basic_common, id_detail),
        InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructMember member = TypeObjectUtils::build_complete_struct_member(id_common, id_detail));
}

// Build CompleteTypeDetail with empty type_name
TEST(TypeObjectUtilsTests, build_complete_type_detail_empty_type_name)
{
    EXPECT_THROW(CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), ""), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name"));
}

/**
 * Auxiliary method that declares extensibility annotations for testing
 */
void complete_type_detail_extensibility(
        CompleteTypeDetail& appendable_extensibility_detail,
        CompleteTypeDetail& final_extensibility_detail,
        CompleteTypeDetail& mutable_extensibility_detail,
        CompleteTypeDetail& appendable_detail,
        CompleteTypeDetail& final_detail,
        CompleteTypeDetail& mutable_detail,
        CompleteTypeDetail& wrong_detail)
{
    AnnotationParameterValue appendable_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        extensibility::ExtensibilityKind::APPENDABLE);
    AnnotationParameterValue final_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        extensibility::ExtensibilityKind::FINAL);
    AnnotationParameterValue mutable_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        extensibility::ExtensibilityKind::MUTABLE);
    NameHash value_hash = TypeObjectUtils::name_hash("value");
    AppliedAnnotationParameter appendable_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        appendable_param_value);
    AppliedAnnotationParameter final_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        final_param_value);
    AppliedAnnotationParameter mutable_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        mutable_param_value);
    AppliedAnnotationParameterSeq appendable_param_seq;
    AppliedAnnotationParameterSeq final_param_seq;
    AppliedAnnotationParameterSeq mutable_param_seq;
    TypeObjectUtils::add_applied_annotation_parameter(appendable_param_seq, appendable_param);
    TypeObjectUtils::add_applied_annotation_parameter(final_param_seq, final_param);
    TypeObjectUtils::add_applied_annotation_parameter(mutable_param_seq, mutable_param);
    TypeIdentifierPair extensibility_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            extensibility_annotation_name, extensibility_type_id));
    TypeIdentifierPair appendable_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            appendable_annotation_name, appendable_type_id));
    TypeIdentifierPair final_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            final_annotation_name, final_type_id));
    TypeIdentifierPair mutable_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            mutable_annotation_name, mutable_type_id));
    AppliedAnnotation appendable_extensibility = TypeObjectUtils::build_applied_annotation(
        extensibility_type_id.type_identifier1(), appendable_param_seq);
    AppliedAnnotation final_extensibility = TypeObjectUtils::build_applied_annotation(
        extensibility_type_id.type_identifier1(), final_param_seq);
    AppliedAnnotation mutable_extensibility = TypeObjectUtils::build_applied_annotation(
        extensibility_type_id.type_identifier1(), mutable_param_seq);
    AppliedAnnotation appendable_ann = TypeObjectUtils::build_applied_annotation(
        appendable_type_id.type_identifier1(), eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    AppliedAnnotation final_ann = TypeObjectUtils::build_applied_annotation(
        final_type_id.type_identifier1(), eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    AppliedAnnotation mutable_ann = TypeObjectUtils::build_applied_annotation(
        mutable_type_id.type_identifier1(), eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    AppliedAnnotationSeq appendable_extensibility_anns;
    AppliedAnnotationSeq final_extensibility_anns;
    AppliedAnnotationSeq mutable_extensibility_anns;
    AppliedAnnotationSeq appendable_anns;
    AppliedAnnotationSeq final_anns;
    AppliedAnnotationSeq mutable_anns;
    AppliedAnnotationSeq wrong_anns;
    TypeObjectUtils::add_applied_annotation(appendable_extensibility_anns, appendable_extensibility);
    TypeObjectUtils::add_applied_annotation(final_extensibility_anns, final_extensibility);
    TypeObjectUtils::add_applied_annotation(mutable_extensibility_anns, mutable_extensibility);
    TypeObjectUtils::add_applied_annotation(appendable_anns, appendable_ann);
    TypeObjectUtils::add_applied_annotation(final_anns, final_ann);
    TypeObjectUtils::add_applied_annotation(mutable_anns, mutable_ann);
    TypeObjectUtils::add_applied_annotation(wrong_anns, appendable_extensibility);
    TypeObjectUtils::add_applied_annotation(wrong_anns, final_ann);
    appendable_extensibility_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), appendable_extensibility_anns, "struct_name");
    final_extensibility_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), final_extensibility_anns, "struct_name");
    mutable_extensibility_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), mutable_extensibility_anns, "struct_name");
    appendable_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), appendable_anns, "struct_name");
    final_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), final_anns, "struct_name");
    mutable_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), mutable_anns, "struct_name");
    wrong_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), wrong_anns, "struct_name");
}

// Build CompleteStructType with inconsistent @extensibility annotation
TEST(TypeObjectUtilsTests, build_complete_struct_type_inconsistent_extensibility)
{
    StructTypeFlag appendable_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    StructTypeFlag final_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::FINAL,
        false, false);
    StructTypeFlag mutable_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::MUTABLE,
        false, false);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "struct_name");
    TypeIdentifier base_type_id;
    CompleteStructHeader basic_header = TypeObjectUtils::build_complete_struct_header(base_type_id, basic_detail);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        basic_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags, basic_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags, basic_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    CompleteTypeDetail appendable_extensibility_detail;
    CompleteTypeDetail final_extensibility_detail;
    CompleteTypeDetail mutable_extensibility_detail;
    CompleteTypeDetail appendable_detail;
    CompleteTypeDetail final_detail;
    CompleteTypeDetail mutable_detail;
    CompleteTypeDetail wrong_detail;
    complete_type_detail_extensibility(appendable_extensibility_detail, final_extensibility_detail,
        mutable_extensibility_detail, appendable_detail, final_detail, mutable_detail, wrong_detail);
    CompleteStructHeader appendable_extensibility_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        appendable_extensibility_detail);
    CompleteStructHeader final_extensibility_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        final_extensibility_detail);
    CompleteStructHeader mutable_extensibility_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        mutable_extensibility_detail);
    CompleteStructHeader appendable_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        appendable_detail);
    CompleteStructHeader final_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        final_detail);
    CompleteStructHeader mutable_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        mutable_detail);
    CompleteStructHeader wrong_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        wrong_detail);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        appendable_extensibility_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        appendable_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        appendable_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        final_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        final_extensibility_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        final_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        mutable_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        mutable_extensibility_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        mutable_extensibility_header, CompleteStructMemberSeq()));
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        appendable_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        appendable_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        appendable_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        final_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        final_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        final_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        mutable_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        mutable_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        mutable_header, CompleteStructMemberSeq()));
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(appendable_flags,
        wrong_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(final_flags,
        wrong_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(mutable_flags,
        wrong_header, CompleteStructMemberSeq()), InvalidArgumentError);
}

// Build CompleteStructType with inconsistent @nested annotation
TEST(TypeObjectUtilsTests, build_complete_struct_type_inconsistent_nested)
{
    StructTypeFlag basic_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    StructTypeFlag nested_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE,
        true, false);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "struct_name");
    TypeIdentifier base_type_id;
    CompleteStructHeader basic_header = TypeObjectUtils::build_complete_struct_header(base_type_id, basic_detail);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(nested_flags, basic_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    AppliedAnnotationSeq anns;
    AppliedAnnotationSeq non_anns;
    applied_annotation_seq_boolean(anns, non_anns, nested_annotation_name);
    CompleteTypeDetail nested_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), anns, "struct_name");
    CompleteTypeDetail non_nested_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), non_anns, "struct_name");
    CompleteStructHeader nested_header = TypeObjectUtils::build_complete_struct_header(base_type_id, nested_detail);
    CompleteStructHeader non_nested_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        non_nested_detail);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(nested_flags,
        non_nested_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(basic_flags, nested_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(nested_flags,
        nested_header, CompleteStructMemberSeq()));
}

/**
 * Auxiliary method to return CompleteTypeDetail with @autoid annotation
 */
void complete_type_detail_autoid(
        CompleteTypeDetail& sequential_detail,
        CompleteTypeDetail& hash_detail,
        CompleteTypeDetail& empty_detail)
{
    AnnotationParameterValue auto_id_sequential_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        autoid::AutoidKind::SEQUENTIAL);
    AnnotationParameterValue auto_id_hash_param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
        autoid::AutoidKind::HASH);
    NameHash value_hash = TypeObjectUtils::name_hash("value");
    AppliedAnnotationParameter auto_id_sequential_param = TypeObjectUtils::build_applied_annotation_parameter(
        value_hash, auto_id_sequential_param_value);
    AppliedAnnotationParameter auto_id_hash_param = TypeObjectUtils::build_applied_annotation_parameter(value_hash,
        auto_id_hash_param_value);
    AppliedAnnotationParameterSeq sequential_param_seq;
    AppliedAnnotationParameterSeq hash_param_seq;
    TypeObjectUtils::add_applied_annotation_parameter(sequential_param_seq, auto_id_sequential_param);
    TypeObjectUtils::add_applied_annotation_parameter(hash_param_seq, auto_id_hash_param);
    TypeIdentifierPair autoid_type_id;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            autoid_annotation_name, autoid_type_id));
    AppliedAnnotation sequential_ann = TypeObjectUtils::build_applied_annotation(autoid_type_id.type_identifier1(),
        sequential_param_seq);
    AppliedAnnotation hash_ann = TypeObjectUtils::build_applied_annotation(autoid_type_id.type_identifier1(),
        hash_param_seq);
    AppliedAnnotation empty_ann = TypeObjectUtils::build_applied_annotation(autoid_type_id.type_identifier1(),
        eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    AppliedAnnotationSeq sequential_anns;
    AppliedAnnotationSeq hash_anns;
    AppliedAnnotationSeq empty_anns;
    TypeObjectUtils::add_applied_annotation(sequential_anns, sequential_ann);
    TypeObjectUtils::add_applied_annotation(hash_anns, hash_ann);
    TypeObjectUtils::add_applied_annotation(empty_anns, empty_ann);
    sequential_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), sequential_anns, "struct_name");
    hash_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), hash_anns, "struct_name");
    empty_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), empty_anns, "struct_name");
}

// Build CompleteStructType with inconsistent @autoid annotation
TEST(TypeObjectUtilsTests, build_complete_struct_type_inconsistent_autoid)
{
    StructTypeFlag basic_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    StructTypeFlag autoid_hash_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE,
        false, true);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "struct_name");
    TypeIdentifier base_type_id;
    CompleteStructHeader basic_header = TypeObjectUtils::build_complete_struct_header(base_type_id, basic_detail);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(autoid_hash_flags,
        basic_header, CompleteStructMemberSeq()), InvalidArgumentError);
    CompleteTypeDetail sequential_detail;
    CompleteTypeDetail hash_detail;
    CompleteTypeDetail empty_detail;
    complete_type_detail_autoid(sequential_detail, hash_detail, empty_detail);
    CompleteStructHeader sequential_header = TypeObjectUtils::build_complete_struct_header(base_type_id,
        sequential_detail);
    CompleteStructHeader hash_header = TypeObjectUtils::build_complete_struct_header(base_type_id, hash_detail);
    CompleteStructHeader empty_header = TypeObjectUtils::build_complete_struct_header(base_type_id, empty_detail);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(autoid_hash_flags,
        sequential_header, CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(basic_flags, hash_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(basic_flags, empty_header,
        CompleteStructMemberSeq()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(autoid_hash_flags,
        hash_header, CompleteStructMemberSeq()));
    EXPECT_NO_THROW(CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(autoid_hash_flags,
        empty_header, CompleteStructMemberSeq()));
}

// Build CommonUnionMember with empty case labels
TEST(TypeObjectUtilsTests, build_common_union_member_empty_case_labels)
{
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, false);
    UnionMemberFlag default_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, true, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    EXPECT_THROW(CommonUnionMember common = TypeObjectUtils::build_common_union_member(0, basic_flags, type_id,
        UnionCaseLabelSeq()), InvalidArgumentError);    
    EXPECT_NO_THROW(CommonUnionMember common = TypeObjectUtils::build_common_union_member(0, default_flags, type_id,
        UnionCaseLabelSeq()));
}

// Build CompleteUnionMember with inconsistent @hashid annotation value and member id
TEST(TypeObjectUtilsTests, build_complete_union_member_inconsistent_hashid_member_id)
{
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonUnionMember common = TypeObjectUtils::build_common_union_member(0x047790DA, basic_flags, type_id,
        {1});
    AppliedBuiltinMemberAnnotations wrong_ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("color"));
    AppliedBuiltinMemberAnnotations ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("shapesize"));
    AppliedBuiltinMemberAnnotations empty_ann_builtin = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>(""));
    CompleteMemberDetail wrong_detail = TypeObjectUtils::build_complete_member_detail("member_name", wrong_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail detail = TypeObjectUtils::build_complete_member_detail("member_name", ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail empty_detail = TypeObjectUtils::build_complete_member_detail("shapesize", empty_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail other_wrong_detail = TypeObjectUtils::build_complete_member_detail("name", empty_ann_builtin,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common, wrong_detail),
        InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common, detail));
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common,
        other_wrong_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common, empty_detail));
}

// Build CompleteUnionMember with inconsistent @try_construct annotation
TEST(TypeObjectUtilsTests, build_complete_union_member_inconsistent_try_construct)
{
    UnionMemberFlag discard_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, false);
    UnionMemberFlag trim_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::TRIM, false, false);
    UnionMemberFlag use_default_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::USE_DEFAULT, false,
        false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonUnionMember discard_common = TypeObjectUtils::build_common_union_member(0, discard_flags, type_id,
        UnionCaseLabelSeq());
    CommonUnionMember trim_common = TypeObjectUtils::build_common_union_member(0, trim_flags, type_id,
        UnionCaseLabelSeq());
    CommonUnionMember use_default_common = TypeObjectUtils::build_common_union_member(0, use_default_flags, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(trim_common,
        empty_member_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(use_default_common,
        empty_member_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(discard_common,
        empty_member_detail));
    CompleteMemberDetail discard_detail;
    CompleteMemberDetail trim_detail;
    CompleteMemberDetail use_default_detail;
    complete_member_detail_try_construct(discard_detail, trim_detail, use_default_detail);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(trim_common,
        discard_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(trim_common,
        trim_detail));
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(trim_common,
        use_default_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(use_default_common,
        discard_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(use_default_common,
        trim_detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(use_default_common,
        use_default_detail));
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(discard_common,
        discard_detail));
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(discard_common,
        trim_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(discard_common,
        use_default_detail), InvalidArgumentError);
}

// Build CompleteUnionMember with inconsistent @external annotation
TEST(TypeObjectUtilsTests, build_complete_union_member_inconsistent_external)
{
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, false);
    UnionMemberFlag external_flag = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, true);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonUnionMember basic_common = TypeObjectUtils::build_common_union_member(0, basic_flags, type_id,
        UnionCaseLabelSeq());
    CommonUnionMember ann_common = TypeObjectUtils::build_common_union_member(0, external_flag, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(ann_common,
        empty_member_detail), InvalidArgumentError);
    CompleteMemberDetail detail;
    CompleteMemberDetail non_detail;
    complete_member_detail_boolean(detail, non_detail, external_annotation_name);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(ann_common,
        non_detail), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(basic_common,
        detail), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(basic_common,
        non_detail));
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(ann_common,
        detail));
}

// Build CompleteUnionMember with inconsistent @id annotation
TEST(TypeObjectUtilsTests, build_complete_union_member_inconsistent_id)
{
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonUnionMember basic_common = TypeObjectUtils::build_common_union_member(0, basic_flags, type_id,
        UnionCaseLabelSeq());
    CommonUnionMember id_common = TypeObjectUtils::build_common_union_member(101, basic_flags, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(id_common,
        empty_member_detail));
    CompleteMemberDetail id_detail;
    complete_member_detail_id(id_detail);
    EXPECT_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(basic_common, id_detail),
        InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(id_common, id_detail));
}

// Build CommonDiscriminatorMember with inconsistent TypeIdentifier
TEST(TypeObjectUtilsTests, build_common_discriminator_member_inconsistent_type_identifier)
{
    UnionDiscriminatorFlag flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD, false);
    TypeIdentifier type_id;
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TK_BOOLEAN);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_BYTE);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_INT8);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_INT16);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_INT32);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_INT64);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_UINT8);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_UINT16);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_UINT32);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_UINT64);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_FLOAT32);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TK_FLOAT64);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TK_FLOAT128);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TK_CHAR8);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    type_id._d(TK_CHAR16);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        type_id));
    StringSTypeDefn small_string = TypeObjectUtils::build_string_s_type_defn(56);
    type_id.string_sdefn(small_string);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TI_STRING16_SMALL);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    StringLTypeDefn large_string = TypeObjectUtils::build_string_l_type_defn(300);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    type_id._d(TI_STRING16_LARGE);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainCollectionHeader collection_header = TypeObjectUtils::build_plain_collection_header(EquivalenceKindValue::BOTH,
        flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id{new TypeIdentifier()};
    primitive_type_id->_d(TK_INT16);
    PlainSequenceSElemDefn small_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(collection_header, 100,
        primitive_type_id);
    type_id.seq_sdefn(small_seq);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainSequenceLElemDefn large_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(collection_header, 1000,
        primitive_type_id);
    type_id.seq_ldefn(large_seq);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainArraySElemDefn small_array = TypeObjectUtils::build_plain_array_s_elem_defn(collection_header,
        {5, 3}, primitive_type_id);
    type_id.array_sdefn(small_array);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainArrayLElemDefn large_array = TypeObjectUtils::build_plain_array_l_elem_defn(collection_header,
        {500, 3}, primitive_type_id);
    type_id.array_ldefn(large_array);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainMapSTypeDefn small_map = TypeObjectUtils::build_plain_map_s_type_defn(collection_header, 100,
        primitive_type_id, flags, primitive_type_id);
    type_id.map_sdefn(small_map);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    PlainMapLTypeDefn large_map = TypeObjectUtils::build_plain_map_l_type_defn(collection_header, 500,
        primitive_type_id, flags, primitive_type_id);
    type_id.map_ldefn(large_map);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
        InvalidArgumentError);
    // Enum
    CommonEnumeratedHeader enum_commom_header = TypeObjectUtils::build_common_enumerated_header(32);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteEnumeratedHeader enum_header = TypeObjectUtils::build_complete_enumerated_header(enum_commom_header,
        empty_type_detail);
    CommonEnumeratedLiteral common_literal = TypeObjectUtils::build_common_enumerated_literal(1, 0);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteEnumeratedLiteral literal = TypeObjectUtils::build_complete_enumerated_literal(common_literal,
        empty_member_detail);
    CompleteEnumeratedLiteralSeq literal_seq;
    TypeObjectUtils::add_complete_enumerated_literal(literal_seq, literal);
    CompleteEnumeratedType enumeration = TypeObjectUtils::build_complete_enumerated_type(0, enum_header, literal_seq);
    TypeObjectUtils::build_and_register_enumerated_type_object(enumeration, "enum");
    TypeIdentifierPair enum_type_identifiers;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
        get_type_identifiers("enum", enum_type_identifiers));
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        enum_type_identifiers.type_identifier1()));
    // Bitmask
    CommonEnumeratedHeader bitmask_common_header = TypeObjectUtils::build_common_enumerated_header(64, true);
    CompleteBitmaskHeader bitmask_header = TypeObjectUtils::build_complete_enumerated_header(bitmask_common_header,
        empty_type_detail, true);
    CompleteBitflagSeq bitflag_seq;
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, CompleteBitflag());
    CompleteBitmaskType bitmask = TypeObjectUtils::build_complete_bitmask_type(0, bitmask_header, bitflag_seq);
    TypeObjectUtils::build_and_register_bitmask_type_object(bitmask, "bitmask");
    TypeIdentifierPair bitmask_type_identifiers;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
        get_type_identifiers("bitmask", bitmask_type_identifiers));
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        bitmask_type_identifiers.type_identifier1()), InvalidArgumentError);
    // Alias: int16_t & float
    CommonAliasBody int16_common_body = TypeObjectUtils::build_common_alias_body(0, *primitive_type_id);
    primitive_type_id->_d(TK_FLOAT32);
    CommonAliasBody float_common_body = TypeObjectUtils::build_common_alias_body(0, *primitive_type_id);
    CompleteAliasBody int16_body = TypeObjectUtils::build_complete_alias_body(int16_common_body,
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteAliasBody float_body = TypeObjectUtils::build_complete_alias_body(float_common_body,
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteAliasHeader alias_header = TypeObjectUtils::build_complete_alias_header(empty_type_detail);
    CompleteAliasType int16_alias = TypeObjectUtils::build_complete_alias_type(0, alias_header, int16_body);
    CompleteAliasType float_alias = TypeObjectUtils::build_complete_alias_type(0, alias_header, float_body);
    TypeObjectUtils::build_and_register_alias_type_object(int16_alias, "alias_int16");
    TypeObjectUtils::build_and_register_alias_type_object(float_alias, "alias_float");
    TypeIdentifierPair alias_type_identifiers;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
        get_type_identifiers("alias_float", alias_type_identifiers));
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        alias_type_identifiers.type_identifier1()), InvalidArgumentError);
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
        get_type_identifiers("alias_int16", alias_type_identifiers));
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
        alias_type_identifiers.type_identifier1()));
}

// Build CompleteDiscriminatorMember with inconsistent @try_construct annotation
TEST(TypeObjectUtilsTests, build_complete_discriminator_member_inconsistent_try_construct)
{
    UnionDiscriminatorFlag discard_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        false);
    UnionDiscriminatorFlag trim_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::TRIM,
        false);
    UnionDiscriminatorFlag use_default_flags = TypeObjectUtils::build_union_discriminator_flag(
        TryConstructKind::USE_DEFAULT, false);
    TypeIdentifier type_id;
    type_id._d(TK_BYTE);
    CommonDiscriminatorMember discard_common_member = TypeObjectUtils::build_common_discriminator_member(discard_flags,
        type_id);
    CommonDiscriminatorMember trim_common_member = TypeObjectUtils::build_common_discriminator_member(trim_flags,
        type_id);
    CommonDiscriminatorMember use_default_common_member = TypeObjectUtils::build_common_discriminator_member(
        use_default_flags, type_id);
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            discard_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()));
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            trim_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            use_default_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    AppliedAnnotationSeq discard_anns;
    AppliedAnnotationSeq trim_anns;
    AppliedAnnotationSeq use_default_anns;
    applied_annotation_seq_try_construct(discard_anns, trim_anns, use_default_anns);
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            discard_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            discard_anns));
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            discard_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            trim_anns), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            discard_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            use_default_anns), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            trim_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            discard_anns), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            trim_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            trim_anns));
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            trim_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            use_default_anns), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            use_default_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            discard_anns), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            use_default_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            trim_anns), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            use_default_common_member, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            use_default_anns));
}

// Build CompleteDiscriminatorMember with inconsistent @key annotation
TEST(TypeObjectUtilsTests, build_complete_discriminator_member_inconsistent_key)
{
    UnionDiscriminatorFlag basic_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        false);
    UnionDiscriminatorFlag key_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        true);
    TypeIdentifier type_id;
    type_id._d(TK_BYTE);
    CommonDiscriminatorMember basic_common = TypeObjectUtils::build_common_discriminator_member(basic_flags, type_id);
    CommonDiscriminatorMember key_common = TypeObjectUtils::build_common_discriminator_member(key_flags, type_id);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            key_common, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    AppliedAnnotationSeq key_annotation;
    AppliedAnnotationSeq non_key_annotation;
    applied_annotation_seq_boolean(key_annotation, non_key_annotation, key_annotation_name);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            basic_common, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            key_annotation), InvalidArgumentError);
    EXPECT_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            key_common, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            non_key_annotation), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            key_common, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            key_annotation));
    EXPECT_NO_THROW(CompleteDiscriminatorMember member = TypeObjectUtils::build_complete_discriminator_member(
            basic_common, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            non_key_annotation));
}

//Build CompleteUnionType with inconsistent @extensibility annotation
TEST(TypeObjectUtilsTests, build_complete_union_type_inconsistent_extensibility)
{
    UnionTypeFlag appendable_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    UnionTypeFlag final_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::FINAL,
        false, false);
    UnionTypeFlag mutable_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::MUTABLE,
        false, false);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "union_name");
    CompleteUnionHeader basic_header = TypeObjectUtils::build_complete_union_header(basic_detail);
    UnionDiscriminatorFlag basic_disc_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        false);
    TypeIdentifier type_id;
    type_id._d(TK_UINT32);
    CommonDiscriminatorMember common_discriminator = TypeObjectUtils::build_common_discriminator_member(
            basic_disc_flags, type_id);
    CompleteDiscriminatorMember discriminator = TypeObjectUtils::build_complete_discriminator_member(
            common_discriminator, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, true,
        false);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(0, member_flags, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, member_detail);
    CompleteUnionMemberSeq members;
    TypeObjectUtils::add_complete_union_member(members, member);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        basic_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags, basic_header,
        discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags, basic_header,
        discriminator, members), InvalidArgumentError);
    CompleteTypeDetail appendable_extensibility_detail;
    CompleteTypeDetail final_extensibility_detail;
    CompleteTypeDetail mutable_extensibility_detail;
    CompleteTypeDetail appendable_detail;
    CompleteTypeDetail final_detail;
    CompleteTypeDetail mutable_detail;
    CompleteTypeDetail wrong_detail;
    complete_type_detail_extensibility(appendable_extensibility_detail, final_extensibility_detail,
        mutable_extensibility_detail, appendable_detail, final_detail, mutable_detail, wrong_detail);
    CompleteUnionHeader appendable_extensibility_header = TypeObjectUtils::build_complete_union_header(
        appendable_extensibility_detail);
    CompleteUnionHeader final_extensibility_header = TypeObjectUtils::build_complete_union_header(
        final_extensibility_detail);
    CompleteUnionHeader mutable_extensibility_header = TypeObjectUtils::build_complete_union_header(
        mutable_extensibility_detail);
    CompleteUnionHeader appendable_header = TypeObjectUtils::build_complete_union_header(
        appendable_detail);
    CompleteUnionHeader final_header = TypeObjectUtils::build_complete_union_header(
        final_detail);
    CompleteUnionHeader mutable_header = TypeObjectUtils::build_complete_union_header(
        mutable_detail);
    CompleteUnionHeader wrong_header = TypeObjectUtils::build_complete_union_header(
        wrong_detail);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        appendable_extensibility_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        appendable_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        appendable_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        final_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        final_extensibility_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        final_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        mutable_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        mutable_extensibility_header, discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        mutable_extensibility_header, discriminator, members));
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        appendable_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        appendable_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        appendable_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        final_header, discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        final_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        final_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        mutable_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        mutable_header, discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        mutable_header, discriminator, members));
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(appendable_flags,
        wrong_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(final_flags,
        wrong_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(mutable_flags,
        wrong_header, discriminator, members), InvalidArgumentError);
}

// Build CompleteUnionType with inconsistent @nested annotation
TEST(TypeObjectUtilsTests, build_complete_union_type_inconsistent_nested)
{
    UnionTypeFlag basic_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    UnionTypeFlag nested_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE,
        true, false);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "union_name");
    CompleteUnionHeader basic_header = TypeObjectUtils::build_complete_union_header(basic_detail);
    UnionDiscriminatorFlag basic_disc_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        false);
    TypeIdentifier type_id;
    type_id._d(TK_UINT32);
    CommonDiscriminatorMember common_discriminator = TypeObjectUtils::build_common_discriminator_member(
            basic_disc_flags, type_id);
    CompleteDiscriminatorMember discriminator = TypeObjectUtils::build_complete_discriminator_member(
            common_discriminator, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, true,
        false);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(0, member_flags, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, member_detail);
    CompleteUnionMemberSeq members;
    TypeObjectUtils::add_complete_union_member(members, member);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(nested_flags, basic_header,
        discriminator, members), InvalidArgumentError);
    AppliedAnnotationSeq anns;
    AppliedAnnotationSeq non_anns;
    applied_annotation_seq_boolean(anns, non_anns, nested_annotation_name);
    CompleteTypeDetail nested_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), anns, "union_name");
    CompleteTypeDetail non_nested_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(), non_anns, "union_name");
    CompleteUnionHeader nested_header = TypeObjectUtils::build_complete_union_header(nested_detail);
    CompleteUnionHeader non_nested_header = TypeObjectUtils::build_complete_union_header(non_nested_detail);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(nested_flags,
        non_nested_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(basic_flags, nested_header,
        discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(nested_flags,
        nested_header, discriminator, members));
}

// Build CompleteUnionType with inconsistent @autoid annotation
TEST(TypeObjectUtilsTests, build_complete_union_type_inconsistent_autoid)
{
    UnionTypeFlag basic_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE,
        false, false);
    UnionTypeFlag autoid_hash_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE,
        false, true);
    CompleteTypeDetail basic_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "union_name");
    CompleteUnionHeader basic_header = TypeObjectUtils::build_complete_union_header(basic_detail);
    UnionDiscriminatorFlag basic_disc_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::DISCARD,
        false);
    TypeIdentifier type_id;
    type_id._d(TK_UINT32);
    CommonDiscriminatorMember common_discriminator = TypeObjectUtils::build_common_discriminator_member(
            basic_disc_flags, type_id);
    CompleteDiscriminatorMember discriminator = TypeObjectUtils::build_complete_discriminator_member(
            common_discriminator, eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::DISCARD, true,
        false);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(0, member_flags, type_id,
        UnionCaseLabelSeq());
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, member_detail);
    CompleteUnionMemberSeq members;
    TypeObjectUtils::add_complete_union_member(members, member);
    CompleteTypeDetail sequential_detail;
    CompleteTypeDetail hash_detail;
    CompleteTypeDetail empty_detail;
    complete_type_detail_autoid(sequential_detail, hash_detail, empty_detail);
    CompleteUnionHeader sequential_header = TypeObjectUtils::build_complete_union_header(sequential_detail);
    CompleteUnionHeader hash_header = TypeObjectUtils::build_complete_union_header(hash_detail);
    CompleteUnionHeader empty_header = TypeObjectUtils::build_complete_union_header(empty_detail);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(autoid_hash_flags,
        sequential_header, discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(basic_flags, hash_header,
        discriminator, members), InvalidArgumentError);
    EXPECT_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(basic_flags, empty_header,
        discriminator, members), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(autoid_hash_flags,
        hash_header, discriminator, members));
    EXPECT_NO_THROW(CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(autoid_hash_flags,
        empty_header, discriminator, members));
}

// Build CommonAnnotationParameter with non-empty flags
TEST(TypeObjectUtilsTests, build_common_annotation_parameter_non_empty_flags)
{
    AnnotationParameterFlag non_empty_flags = 1;
    AnnotationParameterFlag empty_flags = 0;
    TypeIdentifier type_id;
    EXPECT_NO_THROW(type_id._d(TK_INT16));
    EXPECT_THROW(CommonAnnotationParameter common = TypeObjectUtils::build_common_annotation_parameter(non_empty_flags,
        type_id), InvalidArgumentError);
    EXPECT_NO_THROW(CommonAnnotationParameter common = TypeObjectUtils::build_common_annotation_parameter(empty_flags,
        type_id));
}

// Build CompleteAnnotationType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_annotation_type_non_empty_flags)
{
    AnnotationTypeFlag non_empty_flags = 1;
    AnnotationTypeFlag empty_flags = 0;
    CompleteAnnotationHeader header;
    CompleteAnnotationParameterSeq sequence;
    EXPECT_THROW(CompleteAnnotationType annotation = TypeObjectUtils::build_complete_annotation_type(non_empty_flags,
        header, sequence), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteAnnotationType annotation = TypeObjectUtils::build_complete_annotation_type(empty_flags,
        header, sequence));
}

// Build CommonAliasBody with non-empty flags
TEST(TypeObjectUtilsTests, build_common_alias_body_non_empty_flags)
{
    AliasMemberFlag non_empty_flags = 1;
    AliasMemberFlag empty_flags = 0;
    TypeIdentifier type_id;
    EXPECT_NO_THROW(type_id._d(TK_INT16));
    EXPECT_THROW(CommonAliasBody common = TypeObjectUtils::build_common_alias_body(non_empty_flags,
        type_id), InvalidArgumentError);
    EXPECT_NO_THROW(CommonAliasBody common = TypeObjectUtils::build_common_alias_body(empty_flags,
        type_id));
}

// Build CompleteAliasType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_alias_type_non_empty_flags)
{
    AliasTypeFlag non_empty_flags = 1;
    AliasTypeFlag empty_flags = 0;
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "alias_name");
    CompleteAliasHeader header = TypeObjectUtils::build_complete_alias_header(detail);
    TypeIdentifier type_id;
    type_id._d(TK_CHAR8);
    CommonAliasBody common = TypeObjectUtils::build_common_alias_body(0, type_id);
    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> ann_builtin;
    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    CompleteAliasBody body = TypeObjectUtils::build_complete_alias_body(common, ann_builtin, ann_custom);
    EXPECT_THROW(CompleteAliasType alias = TypeObjectUtils::build_complete_alias_type(non_empty_flags,
        header, body), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteAliasType alias = TypeObjectUtils::build_complete_alias_type(empty_flags,
        header, body));
}

// Build CommonCollectionHeader with invalid bound
TEST(TypeObjectUtilsTests, build_common_collection_header_invalid_bound)
{
    LBound bound = 0;
    EXPECT_THROW(CommonCollectionHeader header = TypeObjectUtils::build_common_collection_header(bound),
        InvalidArgumentError);
    bound = 10;
    EXPECT_NO_THROW(CommonCollectionHeader header = TypeObjectUtils::build_common_collection_header(bound));
}

// Build CompleteSequenceType with non-empty flags.
TEST(TypeObjectUtilsTests, build_complete_sequence_type_non_empty_flags)
{
    CollectionTypeFlag non_empty_flags = 1;
    CollectionTypeFlag empty_flags = 0;
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
        eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteElementDetail detail;
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element, detail);
    EXPECT_THROW(CompleteSequenceType sequence = TypeObjectUtils::build_complete_sequence_type(non_empty_flags, header,
        element), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteSequenceType sequence = TypeObjectUtils::build_complete_sequence_type(empty_flags, header,
        element));
}

// Build CommonArrayHeader with invalid bound
TEST(TypeObjectUtilsTests, build_common_array_header_invalid_bound)
{
    LBoundSeq array_bounds;
    EXPECT_THROW(CommonArrayHeader header = TypeObjectUtils::build_common_array_header(array_bounds),
        InvalidArgumentError);
    TypeObjectUtils::add_array_dimension(array_bounds, 150);
    EXPECT_NO_THROW(CommonArrayHeader header = TypeObjectUtils::build_common_array_header(array_bounds));
    TypeObjectUtils::add_array_dimension(array_bounds, 0);
    EXPECT_THROW(CommonArrayHeader header = TypeObjectUtils::build_common_array_header(array_bounds),
        InvalidArgumentError);
}

// Build CompleteArrayType with non-empty flags.
TEST(TypeObjectUtilsTests, build_complete_array_type_non_empty_flags)
{
    CollectionTypeFlag non_empty_flags = 1;
    CollectionTypeFlag empty_flags = 0;
    LBoundSeq array_bounds;
    TypeObjectUtils::add_array_dimension(array_bounds, 356);
    CommonArrayHeader common_header = TypeObjectUtils::build_common_array_header(array_bounds);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "array_type_name");
    CompleteArrayHeader header = TypeObjectUtils::build_complete_array_header(common_header, type_detail);
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteElementDetail detail;
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element, detail);
    EXPECT_THROW(CompleteArrayType array = TypeObjectUtils::build_complete_array_type(non_empty_flags, header,
        element), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteArrayType array = TypeObjectUtils::build_complete_array_type(empty_flags, header,
        element));
}

// Build CompleteMapType with non-empty flags.
TEST(TypeObjectUtilsTests, build_complete_map_type_non_empty_flags)
{
    CollectionTypeFlag non_empty_flags = 1;
    CollectionTypeFlag empty_flags = 0;
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
        eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_INT16);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteElementDetail detail;
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(non_empty_flags, header,
        element, element), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
        element, element));
}

// Build CompleteMapType with inconsistent key TypeIdentifier.
TEST(TypeObjectUtilsTests, build_complete_map_type_inconsistent_key)
{
    CollectionTypeFlag empty_flags = 0;
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
        eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT32);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteElementDetail detail;
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
        element, element), InvalidArgumentError);
    TypeIdentifier key_type_id;
    key_type_id._d(TK_INT32);
    CommonCollectionElement common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    CompleteCollectionElement key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
        key, element));
}

// Build CommonEnumeratedHeader with inconsistent bit bound
TEST(TypeObjectUtilsTests, build_common_enumerated_header_inconsistent_bit_bound)
{
    BitBound bit_bound = 0;
    EXPECT_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound),
        InvalidArgumentError);
    bit_bound = 16;
    EXPECT_NO_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound));
    bit_bound = 33;
    EXPECT_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound),
        InvalidArgumentError);
}

// Build CompleteEnumeratedType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_enumerated_type_non_empty_flags)
{
    EnumTypeFlag empty_flags = 0;
    EnumTypeFlag non_empty_flags = 1;
    CommonEnumeratedHeader common_header = TypeObjectUtils::build_common_enumerated_header(32);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "enumerated_type_name");
    CompleteEnumeratedHeader header = TypeObjectUtils::build_complete_enumerated_header(common_header, type_detail);
    CommonEnumeratedLiteral common_literal = TypeObjectUtils::build_common_enumerated_literal(1, 0);
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("enum_member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteEnumeratedLiteral literal = TypeObjectUtils::build_complete_enumerated_literal(common_literal,
        member_detail);
    CompleteEnumeratedLiteralSeq literal_seq;
    TypeObjectUtils::add_complete_enumerated_literal(literal_seq, literal);
    EXPECT_THROW(CompleteEnumeratedType enumeration = TypeObjectUtils::build_complete_enumerated_type(non_empty_flags,
        header, literal_seq), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteEnumeratedType enumeration = TypeObjectUtils::build_complete_enumerated_type(empty_flags,
        header, literal_seq));
}

// Build CommonBitflag with inconsistent data
TEST(TypeObjectUtilsTests, build_common_bitflag_inconsistent_data)
{
    BitflagFlag empty_flags = 0;
    BitflagFlag non_empty_flags = 1;
    EXPECT_THROW(CommonBitflag bitflag = TypeObjectUtils::build_common_bitflag(65, non_empty_flags),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitflag bitflag = TypeObjectUtils::build_common_bitflag(65, empty_flags),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitflag bitflag = TypeObjectUtils::build_common_bitflag(32, non_empty_flags),
        InvalidArgumentError);
    EXPECT_NO_THROW(CommonBitflag bitflag = TypeObjectUtils::build_common_bitflag(32, empty_flags));
}

// Build CompleteBitmaskType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_bitmask_type_non_empty_flags)
{
    EnumTypeFlag empty_flags = 0;
    EnumTypeFlag non_empty_flags = 1;
    CommonEnumeratedHeader common_header = TypeObjectUtils::build_common_enumerated_header(64, true);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "bitmask_type_name");
    CompleteBitmaskHeader header = TypeObjectUtils::build_complete_enumerated_header(common_header, type_detail, true);
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("bitflag_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitflag bitflag = TypeObjectUtils::build_complete_bitflag(CommonBitflag(), member_detail);
    CompleteBitflagSeq bitflag_seq;
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, bitflag);
    EXPECT_THROW(CompleteBitmaskType bitmask = TypeObjectUtils::build_complete_bitmask_type(non_empty_flags, header,
        bitflag_seq), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteBitmaskType bitmask = TypeObjectUtils::build_complete_bitmask_type(empty_flags, header,
        bitflag_seq));
}

// Build CommonBitfield with inconsistent data
TEST(TypeObjectUtilsTests, build_common_bitfield_inconsistent_data)
{
    BitsetMemberFlag empty_flags = 0;
    BitsetMemberFlag non_empty_flags = 1;
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(65, non_empty_flags, 0, TK_FLOAT128),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(32, non_empty_flags, 0, TK_FLOAT128),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(65, empty_flags, 0, TK_FLOAT128),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(65, non_empty_flags, 3, TK_FLOAT128),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(65, non_empty_flags, 0, TK_BYTE),
        InvalidArgumentError);
    EXPECT_NO_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(32, empty_flags, 3, TK_BYTE));
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(32, empty_flags, 3, TK_BOOLEAN),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(32, empty_flags, 15, TK_BYTE),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(32, empty_flags, 27, TK_UINT16),
        InvalidArgumentError);
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(0, empty_flags, 33, TK_INT32),
        InvalidArgumentError);
    EXPECT_NO_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(0, empty_flags, 64, TK_INT64));
    EXPECT_THROW(CommonBitfield bitfield = TypeObjectUtils::build_common_bitfield(1, empty_flags, 64, TK_UINT64),
        InvalidArgumentError);
}

// Build CompleteBitsetType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_bitset_type_non_empty_flags)
{
    BitsetTypeFlag empty_flags = 0;
    BitsetTypeFlag non_empty_flags = 1;
    CommonBitfield common_bitfield = TypeObjectUtils::build_common_bitfield(0, empty_flags, 3, TK_BYTE);
    CompleteMemberDetail member_detail = TypeObjectUtils::build_complete_member_detail("bitfield_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitfield bitfield = TypeObjectUtils::build_complete_bitfield(common_bitfield, member_detail);
    CompleteBitfieldSeq bitfield_seq;
    TypeObjectUtils::add_complete_bitfield(bitfield_seq, bitfield);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "bitset_type_name");
    CompleteBitsetHeader header = TypeObjectUtils::build_complete_bitset_header(type_detail);
    EXPECT_THROW(CompleteBitsetType bitset = TypeObjectUtils::build_complete_bitset_type(non_empty_flags,
        header, bitfield_seq), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteBitsetType bitset = TypeObjectUtils::build_complete_bitset_type(empty_flags,
        header, bitfield_seq));
}

// Register alias TypeObject
TEST(TypeObjectUtilsTests, register_alias_type_object)
{
    TypeIdentifier related_type;
    related_type._d(TK_CHAR16);
    CommonAliasBody common_body = TypeObjectUtils::build_common_alias_body(0, related_type);
    CompleteAliasBody body = TypeObjectUtils::build_complete_alias_body(common_body,
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteAliasHeader empty_header = TypeObjectUtils::build_complete_alias_header(empty_type_detail);
    CompleteAliasType alias = TypeObjectUtils::build_complete_alias_type(0, empty_header, body);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_alias_type_object(alias, "alias"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_alias_type_object(alias, "alias"));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "alias_name");
    CompleteAliasHeader header = TypeObjectUtils::build_complete_alias_header(detail);
    CompleteAliasType other_alias = TypeObjectUtils::build_complete_alias_type(0, header, body);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_alias_type_object(other_alias,
        "alias"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_alias_type_object(other_alias,
        type_name));
}

// Register annotation TypeObject
TEST(TypeObjectUtilsTests, register_annotation_type_object)
{
    CompleteAnnotationHeader header = TypeObjectUtils::build_complete_annotation_header("annotation_name");
    CompleteAnnotationType annotation = TypeObjectUtils::build_complete_annotation_type(0, header,
        CompleteAnnotationParameterSeq());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(annotation,
        "annotation"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(annotation,
        "annotation"));
    CompleteAnnotationHeader other_header = TypeObjectUtils::build_complete_annotation_header("other_annotation_name");
    CompleteAnnotationType other_annotation = TypeObjectUtils::build_complete_annotation_type(0, other_header,
        CompleteAnnotationParameterSeq());
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_annotation_type_object(
        other_annotation, "annotation"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_annotation_type_object(
        other_annotation, type_name));
}

// Register structure TypeObject
TEST(TypeObjectUtilsTests, register_structure_type_object)
{
    StructTypeFlag flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::APPENDABLE, false, false);
    CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(flags, CompleteStructHeader(),
        CompleteStructMemberSeq());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_struct_type_object(structure,
        "structure"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_struct_type_object(structure,
        "structure"));
    StructTypeFlag other_flags = TypeObjectUtils::build_struct_type_flag(ExtensibilityKind::FINAL, false, false);
    CompleteStructType other_structure = TypeObjectUtils::build_complete_struct_type(other_flags,
        CompleteStructHeader(), CompleteStructMemberSeq());
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_struct_type_object(
        other_structure, "structure"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_struct_type_object(
        other_structure, type_name));
}

// Register union TypeObject
TEST(TypeObjectUtilsTests, register_union_type_object)
{
    UnionTypeFlag flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::APPENDABLE, false, false);
    UnionDiscriminatorFlag discr_flags = TypeObjectUtils::build_union_discriminator_flag(TryConstructKind::USE_DEFAULT,
        false);
    TypeIdentifier discriminator_type_id;
    discriminator_type_id._d(TK_BYTE);
    CommonDiscriminatorMember discr_member = TypeObjectUtils::build_common_discriminator_member(discr_flags,
        discriminator_type_id);
    CompleteDiscriminatorMember discriminator = TypeObjectUtils::build_complete_discriminator_member(discr_member,
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(TryConstructKind::TRIM, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    UnionCaseLabelSeq case_labels;
    TypeObjectUtils::add_union_case_label(case_labels, 5);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(3, member_flags, type_id, case_labels);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, empty_member_detail);
    CompleteUnionMemberSeq member_seq;
    TypeObjectUtils::add_complete_union_member(member_seq, member);
    CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(flags, CompleteUnionHeader(),
        discriminator, member_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_union_type_object(union_type, "union"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_union_type_object(union_type, "union"));
    UnionTypeFlag other_flags = TypeObjectUtils::build_union_type_flag(ExtensibilityKind::MUTABLE, false, false);
    CompleteUnionType other_union_type = TypeObjectUtils::build_complete_union_type(other_flags, CompleteUnionHeader(),
        discriminator, member_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_union_type_object(
        other_union_type, "union"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_union_type_object(
        other_union_type, type_name));
}

// Register bitset TypeObject
TEST(TypeObjectUtilsTests, register_bitset_type_object)
{
    CommonBitfield common_bitfield = TypeObjectUtils::build_common_bitfield(0, 0, 3, TK_BYTE);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitfield bitfield = TypeObjectUtils::build_complete_bitfield(common_bitfield, empty_member_detail);
    CompleteBitfieldSeq bitfield_seq;
    TypeObjectUtils::add_complete_bitfield(bitfield_seq, bitfield);
    CompleteBitsetType bitset = TypeObjectUtils::build_complete_bitset_type(0, CompleteBitsetHeader(), bitfield_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitset_type_object(bitset, "bitset"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitset_type_object(bitset, "bitset"));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "bitset");
    CompleteBitsetHeader header = TypeObjectUtils::build_complete_bitset_header(detail);
    CompleteBitsetType other_bitset = TypeObjectUtils::build_complete_bitset_type(0, header, bitfield_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_bitset_type_object(
        other_bitset, "bitset"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_bitset_type_object(
        other_bitset, type_name));
}

// Register sequence TypeObject
TEST(TypeObjectUtilsTests, register_sequence_type_object)
{
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
        eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
        CompleteElementDetail());
    CompleteSequenceType sequence = TypeObjectUtils::build_complete_sequence_type(0, header, element);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_sequence_type_object(sequence, "sequence"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_sequence_type_object(sequence, "sequence"));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "sequence");
    CompleteCollectionHeader other_header = TypeObjectUtils::build_complete_collection_header(common_header, detail);
    CompleteSequenceType other_sequence = TypeObjectUtils::build_complete_sequence_type(0, other_header, element);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_sequence_type_object(
        other_sequence, "sequence"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_sequence_type_object(
        other_sequence, type_name));
}

// Register array TypeObject
TEST(TypeObjectUtilsTests, register_array_type_object)
{
    LBoundSeq array_bounds;
    TypeObjectUtils::add_array_dimension(array_bounds, 356);
    CommonArrayHeader common_header = TypeObjectUtils::build_common_array_header(array_bounds);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteArrayHeader header = TypeObjectUtils::build_complete_array_header(common_header, empty_type_detail);
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
        CompleteElementDetail());
    CompleteArrayType array = TypeObjectUtils::build_complete_array_type(0, header, element);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_array_type_object(array, "array"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_array_type_object(array, "array"));
    type_id._d(TK_INT16);
    CommonCollectionElement other_common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement other_element = TypeObjectUtils::build_complete_collection_element(other_common_element,
        CompleteElementDetail());
    CompleteArrayType other_array = TypeObjectUtils::build_complete_array_type(0, header, other_element);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_array_type_object(other_array,
        "array"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_array_type_object(other_array,
        type_name));
}

// Register map TypeObject
TEST(TypeObjectUtilsTests, register_map_type_object)
{
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
        eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(TryConstructKind::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_INT16);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
        CompleteElementDetail());
    CompleteMapType map = TypeObjectUtils::build_complete_map_type(0, header, element, element);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_map_type_object(map, "map"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_map_type_object(map, "map"));
    type_id._d(TK_INT32);
    CommonCollectionElement key_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement key = TypeObjectUtils::build_complete_collection_element(key_element,
        CompleteElementDetail());
    CompleteMapType other_map = TypeObjectUtils::build_complete_map_type(0, header, key, element);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_map_type_object(other_map,
        "map"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_map_type_object(other_map,
        type_name));
}

// Register enumeration TypeObject
TEST(TypeObjectUtilsTests, register_enumerated_type_object)
{
    CommonEnumeratedHeader common_header = TypeObjectUtils::build_common_enumerated_header(32);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteEnumeratedHeader header = TypeObjectUtils::build_complete_enumerated_header(common_header,
        empty_type_detail);
    CommonEnumeratedLiteral common_literal = TypeObjectUtils::build_common_enumerated_literal(1, 0);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteEnumeratedLiteral literal = TypeObjectUtils::build_complete_enumerated_literal(common_literal,
        empty_member_detail);
    CompleteEnumeratedLiteralSeq literal_seq;
    TypeObjectUtils::add_complete_enumerated_literal(literal_seq, literal);
    CompleteEnumeratedType enumeration = TypeObjectUtils::build_complete_enumerated_type(0, header, literal_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_enumerated_type_object(enumeration,
        "enum"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_enumerated_type_object(enumeration,
        "enum"));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "enumeration");
    CompleteEnumeratedHeader other_header = TypeObjectUtils::build_complete_enumerated_header(common_header, detail);
    CompleteEnumeratedType other_enumeration = TypeObjectUtils::build_complete_enumerated_type(0, other_header,
        literal_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_enumerated_type_object(other_enumeration,
        "enum"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_enumerated_type_object(other_enumeration,
        type_name));
}

// Register bitmask TypeObject
TEST(TypeObjectUtilsTests, register_bitmask_type_object)
{
    CommonEnumeratedHeader common_header = TypeObjectUtils::build_common_enumerated_header(64, true);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
            eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
            eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteBitmaskHeader header = TypeObjectUtils::build_complete_enumerated_header(common_header, type_detail, true);
    CompleteBitflag bitflag;
    CompleteBitflagSeq bitflag_seq;
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, bitflag);
    CompleteBitmaskType bitmask = TypeObjectUtils::build_complete_bitmask_type(0, header, bitflag_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitmask_type_object(bitmask, "bitmask"));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitmask_type_object(bitmask, "bitmask"));
    CommonBitflag common = TypeObjectUtils::build_common_bitflag(1, 0);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitflag other_bitflag = TypeObjectUtils::build_complete_bitflag(common, empty_member_detail);
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, other_bitflag);
    CompleteBitmaskType other_bitmask = TypeObjectUtils::build_complete_bitmask_type(0, header, bitflag_seq);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitmask_type_object(other_bitmask,
        "bitmask"));
    std::string type_name;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, TypeObjectUtils::build_and_register_bitmask_type_object(other_bitmask,
        type_name));
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
