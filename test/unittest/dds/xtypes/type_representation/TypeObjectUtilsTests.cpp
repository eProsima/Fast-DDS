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

#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

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

// Build inconsistent StructMemberFlag
TEST(TypeObjectUtilsTests, build_inconsistent_struct_member_flag)
{
    EXPECT_THROW(TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::
                    DISCARD,
            true, false, true, false), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::
                    DISCARD,
            true, false, false, false));
    EXPECT_NO_THROW(TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::
                    DISCARD,
            false, false, true, false));
}

// Build StringLTypeDefn with bound smaller than 256.
TEST(TypeObjectUtilsTests, build_string_l_type_defn_small_bound)
{
    LBound wrong_bound = 255;
    EXPECT_THROW(StringLTypeDefn string_l_type_defn = TypeObjectUtils::build_string_l_type_defn(wrong_bound),
            InvalidArgumentError);
    EXPECT_NO_THROW(StringLTypeDefn string_l_type_defn = TypeObjectUtils::build_string_l_type_defn(256));
}

void register_empty_structure_type_object()
{
    std::string empty_struct_name = "empty_structure";
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), empty_struct_name);
    CompleteStructHeader header = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail);
    CompleteStructType struct_type = TypeObjectUtils::build_complete_struct_type(0, header, CompleteStructMemberSeq());
    TypeIdentifierPair type_ids;
    TypeObjectUtils::build_and_register_struct_type_object(struct_type, empty_struct_name, type_ids);
}

void register_plain_seq_type_object(
        eprosima::fastcdr::external<TypeIdentifier> complete_typeid)
{
    std::string plain_seq_name = "plain_sequence";
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_COMPLETE, flags);
    PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        header, 255, complete_typeid);
    TypeIdentifierPair type_ids;
    TypeObjectUtils::build_and_register_s_sequence_type_identifier(plain_seq, plain_seq_name, type_ids);
}

void register_alias_type_object()
{
    TypeIdentifier primitive_type_id;
    primitive_type_id._d(TK_INT16);
    CommonAliasBody int16_common_body = TypeObjectUtils::build_common_alias_body(0, primitive_type_id);
    CompleteAliasBody int16_body = TypeObjectUtils::build_complete_alias_body(int16_common_body,
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteAliasHeader alias_header = TypeObjectUtils::build_complete_alias_header(empty_type_detail);
    CompleteAliasType alias_type = TypeObjectUtils::build_complete_alias_type(0, alias_header, int16_body);
    TypeIdentifierPair type_ids;
    TypeObjectUtils::build_and_register_alias_type_object(alias_type, "int16_alias", type_ids);
}

// Build PlainSequenceSElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_sequence_s_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
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
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
                minimal_header, 10, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
                fully_descriptive_header, 10, test_identifier));
    // Change discriminator to EK_COMPLETE
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
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
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
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
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
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
    wrong_bound = 0;
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
                complete_header, wrong_bound, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
                complete_header, 256, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
                minimal_header, 256, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
                fully_descriptive_header, 256, test_identifier));
    // Change discriminator to EK_COMPLETE
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
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
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
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

// Add inconsistent dimension to array
TEST(TypeObjectUtilsTests, add_inconsistent_array_dimension)
{
    SBoundSeq small_dimensions;
    LBoundSeq large_dimensions;
    SBound invalid_sbound = 0;
    LBound invalid_lbound = 0;
    EXPECT_THROW(TypeObjectUtils::add_array_dimension(small_dimensions, invalid_sbound), InvalidArgumentError);
    EXPECT_THROW(TypeObjectUtils::add_array_dimension(large_dimensions, invalid_lbound), InvalidArgumentError);
}

// Build PlainArraySElemDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_array_s_elem_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    SBoundSeq bound_seq;
    SBound bound = 10;
    TypeObjectUtils::add_array_dimension(bound_seq, bound);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
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
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, bound);
    bound = 0;
    EXPECT_THROW(TypeObjectUtils::add_array_dimension(wrong_bound_seq, bound), InvalidArgumentError);
    wrong_bound_seq.push_back(0);
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
                complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
                complete_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
                minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(
                fully_descriptive_header, bound_seq, test_identifier));
    // Change discriminator to EK_COMPLETE
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
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
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
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
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    LBoundSeq bound_seq;
    LBound bound = 256;
    TypeObjectUtils::add_array_dimension(bound_seq, bound);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
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
    bound = 10;
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, bound);
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                complete_header, wrong_bound_seq, test_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                complete_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                minimal_header, bound_seq, test_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
    // TypeIdentifier consistent with fully-descriptive header
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                fully_descriptive_header, bound_seq, test_identifier));
    // At least one dimension should be large
    bound = 256;
    TypeObjectUtils::add_array_dimension(wrong_bound_seq, bound);
    EXPECT_NO_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                fully_descriptive_header, wrong_bound_seq, test_identifier));
    // Zero element
    bound = 0;
    EXPECT_THROW(TypeObjectUtils::add_array_dimension(wrong_bound_seq, bound), InvalidArgumentError);
    wrong_bound_seq.push_back(0);
    EXPECT_THROW(PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(
                fully_descriptive_header, wrong_bound_seq, test_identifier), InvalidArgumentError);
    // Change discriminator to EK_COMPLETE
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
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
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        test_identifier = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }
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
TEST(TypeObjectUtilsTests, build_plain_map_s_type_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
#if !defined(NDEBUG)
    PlainCollectionHeader wrong_header;
    // Inconsistent header CollectionElementFlags
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                wrong_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    wrong_header.element_flags(flags);
    // Inconsistent header EquivalenceKind
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                wrong_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // Non-initialized TypeIdentifier
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Check SBound consistency
    SBound wrong_bound = 0;
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, wrong_bound, test_identifier, flags, key_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
    // Wrong key_flags
    CollectionElementFlag wrong_flags = 0;
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, wrong_flags, key_identifier), InvalidArgumentError);
#if !defined(NDEBUG)
    // Uninitialized key_identifier
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
#endif // !defined(NDEBUG)
    // Non-integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_FLOAT32));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    // TypeIdentifier consistent with fully-descriptive header and integer key identifier
    EXPECT_NO_THROW(key_identifier->_d(TK_INT64));
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, key_identifier));
    StringSTypeDefn string_type_def = TypeObjectUtils::build_string_s_type_defn(50);
    EXPECT_NO_THROW(key_identifier->string_sdefn(string_type_def));
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, key_identifier));

    // Get EK_COMPLETE and EK_MINIMAL TypeIdentifiers
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    eprosima::fastcdr::external<TypeIdentifier> complete_typeid;
    eprosima::fastcdr::external<TypeIdentifier> minimal_typeid;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    register_plain_seq_type_object(complete_typeid);
    eprosima::fastcdr::external<TypeIdentifier> complete_indirect_id;
    eprosima::fastcdr::external<TypeIdentifier> minimal_indirect_id;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("plain_sequence", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    register_alias_type_object();
    eprosima::fastcdr::external<TypeIdentifier> complete_alias_id;
    eprosima::fastcdr::external<TypeIdentifier> minimal_alias_id;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("int16_alias", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, test_identifier, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                fully_descriptive_header, 10, minimal_alias_id, flags, minimal_alias_id), InvalidArgumentError);


    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key///
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key///
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, complete_alias_id));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, test_identifier, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, complete_alias_id));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, complete_alias_id));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, complete_alias_id));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                complete_header, 10, minimal_alias_id, flags, minimal_alias_id), InvalidArgumentError);


    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, test_identifier, flags, minimal_alias_id));

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_typeid, flags, minimal_alias_id));

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_indirect_id, flags, minimal_alias_id));

    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, key_identifier));
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(
                minimal_header, 10, minimal_alias_id, flags, minimal_alias_id));
}

// Build PlainMapLTypeDefn with inconsistent parameters.
TEST(TypeObjectUtilsTests, build_plain_map_l_type_defn_inconsistencies)
{
    eprosima::fastcdr::external<TypeIdentifier> test_identifier{new TypeIdentifier()};
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::TRIM, false);
    PlainCollectionHeader complete_header = TypeObjectUtils::build_plain_collection_header(
        EK_COMPLETE, flags);
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
    wrong_bound = 0;
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, wrong_bound, test_identifier, flags, key_identifier), InvalidArgumentError);

    // Primitive TypeIdentifier
    EXPECT_NO_THROW(test_identifier->_d(TK_BOOLEAN));
    // TypeIdentifier inconsistent with complete header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader minimal_header = TypeObjectUtils::build_plain_collection_header(
        EK_MINIMAL, flags);
    // TypeIdentifier inconsistent with minimal header
    EXPECT_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    PlainCollectionHeader fully_descriptive_header = TypeObjectUtils::build_plain_collection_header(
        EK_BOTH, flags);
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
    StringSTypeDefn string_type_def = TypeObjectUtils::build_string_s_type_defn(50);
    EXPECT_NO_THROW(key_identifier->string_sdefn(string_type_def));
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_seq = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, key_identifier));



    // Get EK_COMPLETE and EK_MINIMAL TypeIdentifiers
    register_empty_structure_type_object();
    TypeIdentifierPair type_ids;
    eprosima::fastcdr::external<TypeIdentifier> complete_typeid;
    eprosima::fastcdr::external<TypeIdentifier> minimal_typeid;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("empty_structure", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_typeid = eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    register_plain_seq_type_object(complete_typeid);
    eprosima::fastcdr::external<TypeIdentifier> complete_indirect_id;
    eprosima::fastcdr::external<TypeIdentifier> minimal_indirect_id;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("plain_sequence", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_indirect_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    register_alias_type_object();
    eprosima::fastcdr::external<TypeIdentifier> complete_alias_id;
    eprosima::fastcdr::external<TypeIdentifier> minimal_alias_id;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("int16_alias", type_ids);
    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        complete_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
        minimal_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
    }
    else
    {
        complete_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier2()));
        minimal_alias_id =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(type_ids.type_identifier1()));
    }

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, test_identifier, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, complete_indirect_id),
            InvalidArgumentError);                                                                                         //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, minimal_indirect_id),
            InvalidArgumentError);                                                                                        //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, complete_indirect_id),
            InvalidArgumentError);                                                                                        //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                fully_descriptive_header, 1000, minimal_alias_id, flags, minimal_alias_id), InvalidArgumentError);


    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key///
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key///
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, complete_alias_id));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, test_identifier, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, complete_alias_id));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, complete_alias_id));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, complete_alias_id));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                complete_header, 1000, minimal_alias_id, flags, minimal_alias_id), InvalidArgumentError);


    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, test_identifier, flags, minimal_alias_id));

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_typeid, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_indirect_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, key_identifier), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, complete_alias_id, flags, minimal_alias_id), InvalidArgumentError);

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_typeid, flags, minimal_alias_id));

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_indirect_id, flags, minimal_alias_id));

    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, key_identifier));
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, complete_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, complete_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, complete_alias_id), InvalidArgumentError);
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, minimal_typeid), InvalidArgumentError); //Invalid key
    EXPECT_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, minimal_indirect_id), InvalidArgumentError); //Invalid key
    EXPECT_NO_THROW(PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(
                minimal_header, 1000, minimal_alias_id, flags, minimal_alias_id));
}

// Register small string/wstring. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_string)
{
    TypeIdentifierPair type_ids;
    StringSTypeDefn string_defn = TypeObjectUtils::build_string_s_type_defn(32);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_string_type_identifier(string_defn, "small_string", type_ids));
    // Registering twice the same TypeIdentifier should not fail
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_string_type_identifier(string_defn, "small_string", type_ids));
    // Registering another TypeIdentifier with the same name should return RETCODE_BAD_PARAMETER
    StringSTypeDefn another_string_defn = TypeObjectUtils::build_string_s_type_defn(100);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_string_type_identifier(
                another_string_defn, "small_string",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_s_string_type_identifier(
                another_string_defn, type_name,
                type_ids));
}

// Register large string/wstring. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_string)
{
    TypeIdentifierPair type_ids;
    StringLTypeDefn string_defn = TypeObjectUtils::build_string_l_type_defn(1000);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_string_type_identifier(string_defn, "large_string", type_ids));
    // Registering twice the same TypeIdentifier should not fail
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_string_type_identifier(string_defn, "large_string", type_ids));
    // Registering another TypeIdentifier with the same name should return RETCODE_BAD_PARAMETER
    StringLTypeDefn another_string_defn = TypeObjectUtils::build_string_l_type_defn(2000);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_string_type_identifier(
                another_string_defn, "large_string",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_l_string_type_identifier(
                another_string_defn, type_name,
                type_ids));
}

// Register small sequence. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_sequence)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    PlainSequenceSElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        header, 255, primitive_identifier);
    // Another external is required cause the comparison is only of the pointer and not the data contained.
    eprosima::fastcdr::external<TypeIdentifier> other_identifier{new TypeIdentifier()};
    other_identifier->_d(TK_INT16);
    PlainSequenceSElemDefn another_plain_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(
        header, 255, other_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_sequence_type_identifier(plain_seq, "small_sequence", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_sequence_type_identifier(plain_seq, "small_sequence", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_sequence_type_identifier(
                another_plain_seq,
                "small_sequence",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            TypeObjectUtils::build_and_register_s_sequence_type_identifier(another_plain_seq, type_name, type_ids));
}

// Register large sequence. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_sequence)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    PlainSequenceLElemDefn plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        header, 256, primitive_identifier);
    eprosima::fastcdr::external<TypeIdentifier> other_identifier{new TypeIdentifier()};
    other_identifier->_d(TK_INT16);
    PlainSequenceLElemDefn another_plain_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(
        header, 256, other_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_sequence_type_identifier(plain_seq, "large_sequence", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_sequence_type_identifier(plain_seq, "large_sequence", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_sequence_type_identifier(
                another_plain_seq, "large_sequence",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            TypeObjectUtils::build_and_register_l_sequence_type_identifier(another_plain_seq, type_name, type_ids));
}

// Register small array. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_array)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    SBoundSeq array_bounds;
    SBound bound = 26;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    PlainArraySElemDefn plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(header, array_bounds,
                    primitive_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_array_type_identifier(plain_array, "small_array", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_s_array_type_identifier(plain_array, "small_array", type_ids));
    bound = 100;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    PlainArraySElemDefn another_plain_array = TypeObjectUtils::build_plain_array_s_elem_defn(header, array_bounds,
                    primitive_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_array_type_identifier(
                another_plain_array, "small_array",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_s_array_type_identifier(
                another_plain_array,
                type_name,
                type_ids));
}

// Register large array. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_array)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_FLOAT128);
    LBoundSeq array_bounds;
    LBound bound = 260;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    PlainArrayLElemDefn plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(header, array_bounds,
                    primitive_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_array_type_identifier(plain_array, "large_array", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_l_array_type_identifier(plain_array, "large_array", type_ids));
    bound = 1000;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    PlainArrayLElemDefn another_plain_array = TypeObjectUtils::build_plain_array_l_elem_defn(header, array_bounds,
                    primitive_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_array_type_identifier(
                another_plain_array, "large_array",
                type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_l_array_type_identifier(
                another_plain_array, type_name,
                type_ids));
}

// Register small map. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_s_map)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_UINT32);
    PlainMapSTypeDefn plain_map = TypeObjectUtils::build_plain_map_s_type_defn(header, 10, primitive_identifier, flags,
                    primitive_identifier);
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    key_identifier->_d(TK_INT8);
    PlainMapSTypeDefn another_plain_map = TypeObjectUtils::build_plain_map_s_type_defn(header, 10, primitive_identifier,
                    flags, key_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_s_map_type_identifier(plain_map,
            "small_map", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_s_map_type_identifier(plain_map,
            "small_map", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_s_map_type_identifier(
                another_plain_map, "small_map", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_s_map_type_identifier(
                another_plain_map,
                type_name,
                type_ids));
}

// Register large map. This test does not check member consistency (only checked in Debug build mode).
TEST(TypeObjectUtilsTests, register_l_map)
{
    TypeIdentifierPair type_ids;
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, false);
    PlainCollectionHeader header = TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
    eprosima::fastcdr::external<TypeIdentifier> primitive_identifier{new TypeIdentifier()};
    primitive_identifier->_d(TK_UINT32);
    PlainMapLTypeDefn plain_map = TypeObjectUtils::build_plain_map_l_type_defn(header, 500, primitive_identifier, flags,
                    primitive_identifier);
    eprosima::fastcdr::external<TypeIdentifier> key_identifier{new TypeIdentifier()};
    key_identifier->_d(TK_INT8);
    PlainMapLTypeDefn other_plain_map = TypeObjectUtils::build_plain_map_l_type_defn(header, 500, primitive_identifier,
                    flags, key_identifier);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_l_map_type_identifier(plain_map,
            "large_map", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_l_map_type_identifier(plain_map,
            "large_map", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_l_map_type_identifier(
                other_plain_map, "large_map", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_l_map_type_identifier(
                other_plain_map, type_name,
                type_ids));
}

// Build AppliedAnnotation invalid TypeIdentifier
TEST(TypeObjectUtilsTests, build_applied_annotation_invalid_type_identifier)
{
    TypeIdentifierPair type_ids;
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    EXPECT_THROW(AppliedAnnotation annotation = TypeObjectUtils::build_applied_annotation(type_id,
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>()), InvalidArgumentError);
    CompleteAnnotationHeader annotation_header = TypeObjectUtils::build_complete_annotation_header("custom_annotation");
    CompleteAnnotationType custom_annotation = TypeObjectUtils::build_complete_annotation_type(0, annotation_header,
                    CompleteAnnotationParameterSeq());
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_annotation_type_object(custom_annotation, "custom", type_ids));
    TypeIdentifierPair custom_annotation_ids;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("custom",
            custom_annotation_ids));
    EXPECT_NO_THROW(AppliedAnnotation annotation = TypeObjectUtils::build_applied_annotation(
                custom_annotation_ids.type_identifier1(),
                eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>()));
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
    StructMemberFlag basic_flags = TypeObjectUtils::build_struct_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false,
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

// Build CommonUnionMember with empty case labels
TEST(TypeObjectUtilsTests, build_common_union_member_empty_case_labels)
{
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false);
    UnionMemberFlag default_flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, true, false);
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
    UnionMemberFlag basic_flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false);
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

/**
 * Auxiliary methods to build valid hash TypeIdentifiers
 */
void small_string_type_identifier(
        TypeIdentifier& type_id)
{
    StringSTypeDefn small_string = TypeObjectUtils::build_string_s_type_defn(56);
    type_id.string_sdefn(small_string);
}

void large_string_type_identifier(
        TypeIdentifier& type_id)
{
    StringLTypeDefn large_string = TypeObjectUtils::build_string_l_type_defn(300);
    type_id.string_ldefn(large_string);
}

const PlainCollectionHeader plain_collection_header()
{
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false);
    return TypeObjectUtils::build_plain_collection_header(EK_BOTH, flags);
}

const eprosima::fastcdr::external<TypeIdentifier> primitive_type_identifier()
{
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id{new TypeIdentifier()};
    primitive_type_id->_d(TK_INT16);
    return primitive_type_id;
}

void small_sequence_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    PlainSequenceSElemDefn small_seq = TypeObjectUtils::build_plain_sequence_s_elem_defn(collection_header, 100,
                    primitive_type_id);
    type_id.seq_sdefn(small_seq);
}

void large_sequence_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    PlainSequenceLElemDefn large_seq = TypeObjectUtils::build_plain_sequence_l_elem_defn(collection_header, 1000,
                    primitive_type_id);
    type_id.seq_ldefn(large_seq);
}

void small_array_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    PlainArraySElemDefn small_array = TypeObjectUtils::build_plain_array_s_elem_defn(collection_header,
                    {5, 3}, primitive_type_id);
    type_id.array_sdefn(small_array);
}

void large_array_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    PlainArrayLElemDefn large_array = TypeObjectUtils::build_plain_array_l_elem_defn(collection_header,
                    {500, 3}, primitive_type_id);
    type_id.array_ldefn(large_array);
}

void small_map_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false);
    PlainMapSTypeDefn small_map = TypeObjectUtils::build_plain_map_s_type_defn(collection_header, 100,
                    primitive_type_id, flags, primitive_type_id);
    type_id.map_sdefn(small_map);
}

void large_map_type_identifier(
        TypeIdentifier& type_id)
{
    PlainCollectionHeader collection_header = plain_collection_header();
    eprosima::fastcdr::external<TypeIdentifier> primitive_type_id = primitive_type_identifier();
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false);
    PlainMapLTypeDefn large_map = TypeObjectUtils::build_plain_map_l_type_defn(collection_header, 500,
                    primitive_type_id, flags, primitive_type_id);
    type_id.map_ldefn(large_map);
}

const CompleteAliasType int_16_alias()
{
    TypeIdentifier primitive_type_id;
    primitive_type_id._d(TK_INT16);
    CommonAliasBody int16_common_body = TypeObjectUtils::build_common_alias_body(0, primitive_type_id);
    CompleteAliasBody int16_body = TypeObjectUtils::build_complete_alias_body(int16_common_body,
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteAliasHeader alias_header = TypeObjectUtils::build_complete_alias_header(empty_type_detail);
    return TypeObjectUtils::build_complete_alias_type(0, alias_header, int16_body);
}

const CompleteAliasType float32_alias()
{
    TypeIdentifier primitive_type_id;
    primitive_type_id._d(TK_FLOAT32);
    CommonAliasBody float_common_body = TypeObjectUtils::build_common_alias_body(0, primitive_type_id);
    CompleteAliasBody float_body = TypeObjectUtils::build_complete_alias_body(float_common_body,
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteAliasHeader alias_header = TypeObjectUtils::build_complete_alias_header(empty_type_detail);
    return TypeObjectUtils::build_complete_alias_type(0, alias_header, float_body);
}

void register_basic_enum()
{
    TypeIdentifierPair type_ids;
    std::string basic_enum_name = "basic_enum";
    CommonEnumeratedHeader common = TypeObjectUtils::build_common_enumerated_header(32);
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), basic_enum_name);
    CompleteEnumeratedHeader header = TypeObjectUtils::build_complete_enumerated_header(common, detail);
    CommonEnumeratedLiteral common_literal = TypeObjectUtils::build_common_enumerated_literal(1, 0);
    CompleteMemberDetail detail_literal = TypeObjectUtils::build_complete_member_detail("literal",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteEnumeratedLiteral literal = TypeObjectUtils::build_complete_enumerated_literal(common_literal,
                    detail_literal);
    CompleteEnumeratedLiteralSeq literal_seq;
    TypeObjectUtils::add_complete_enumerated_literal(literal_seq, literal);
    CompleteEnumeratedType enum_type = TypeObjectUtils::build_complete_enumerated_type(0, header, literal_seq);
    TypeObjectUtils::build_and_register_enumerated_type_object(enum_type, basic_enum_name, type_ids);
}

void register_basic_bitmask()
{
    TypeIdentifierPair type_ids;
    std::string basic_bitmask_name = "basic_bitmask";
    CommonEnumeratedHeader common = TypeObjectUtils::build_common_enumerated_header(32, true);
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), basic_bitmask_name);
    CompleteEnumeratedHeader header = TypeObjectUtils::build_complete_enumerated_header(common, detail, true);
    CommonBitflag common_flag = TypeObjectUtils::build_common_bitflag(5, 0);
    CompleteMemberDetail detail_flag = TypeObjectUtils::build_complete_member_detail("flag",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitflag flag = TypeObjectUtils::build_complete_bitflag(common_flag, detail_flag);
    CompleteBitflagSeq flag_seq;
    TypeObjectUtils::add_complete_bitflag(flag_seq, flag);
    CompleteBitmaskType bitmask_type = TypeObjectUtils::build_complete_bitmask_type(0, header, flag_seq);
    TypeObjectUtils::build_and_register_bitmask_type_object(bitmask_type, basic_bitmask_name, type_ids);
}

// Build CommonDiscriminatorMember with inconsistent TypeIdentifier
TEST(TypeObjectUtilsTests, build_common_discriminator_member_inconsistent_type_identifier)
{
    UnionDiscriminatorFlag flags = TypeObjectUtils::build_union_discriminator_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false);
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
    small_string_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    type_id._d(TI_STRING16_SMALL);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    large_string_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    type_id._d(TI_STRING16_LARGE);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    small_sequence_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    large_sequence_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    small_array_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    large_array_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    small_map_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    large_map_type_identifier(type_id);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags, type_id),
            InvalidArgumentError);
    // Enum
    register_basic_enum();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_enum", type_ids);
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
            type_ids.type_identifier1()));
    // Bitmask
    register_basic_bitmask();
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_bitmask", type_ids);
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
            type_ids.type_identifier1()), InvalidArgumentError);
    // Alias: int16_t & float
    CompleteAliasType int16_alias = int_16_alias();
    CompleteAliasType float_alias = float32_alias();
    TypeObjectUtils::build_and_register_alias_type_object(int16_alias, "alias_int16", type_ids);
    TypeObjectUtils::build_and_register_alias_type_object(float_alias, "alias_float", type_ids);
    TypeIdentifierPair alias_type_identifiers;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers("alias_float", alias_type_identifiers));
    EXPECT_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
            alias_type_identifiers.type_identifier1()), InvalidArgumentError);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers("alias_int16", alias_type_identifiers));
    EXPECT_NO_THROW(CommonDiscriminatorMember member = TypeObjectUtils::build_common_discriminator_member(flags,
            alias_type_identifiers.type_identifier1()));
}

// Add CompleteUnionMember to sequence with invalid name
TEST(TypeObjectUtilsTests, build_complete_union_member_invalid_name)
{
    std::string invalid_name = "discriminator";
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    UnionCaseLabelSeq case_labels;
    TypeObjectUtils::add_union_case_label(case_labels, 5);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(3, member_flags, type_id, case_labels);
    CompleteMemberDetail invalid_member_detail = TypeObjectUtils::build_complete_member_detail(invalid_name,
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, invalid_member_detail);
    CompleteUnionMemberSeq member_seq;
    EXPECT_THROW(TypeObjectUtils::add_complete_union_member(member_seq, member), InvalidArgumentError);
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

/**
 * Auxiliary method to check annotation parameter consistency
 */
void check_annotation_parameter_consistency(
        const CommonAnnotationParameter& common,
        const std::vector<AnnotationParameterValue>& ann_param_seq,
        const std::vector<bool>& expected_results)
{
    for (size_t i = 0; i < ann_param_seq.size(); i++)
    {
        if (expected_results[i])
        {
            EXPECT_NO_THROW(CompleteAnnotationParameter param = TypeObjectUtils::build_complete_annotation_parameter(
                        common, "param_name", ann_param_seq[i]));
        }
        else
        {
            EXPECT_THROW(CompleteAnnotationParameter param = TypeObjectUtils::build_complete_annotation_parameter(
                        common, "param_name", ann_param_seq[i]), InvalidArgumentError);
        }
    }
}

// Build CompleteAnnotationParameter with inconsistent TypeIdentifier and AnnotationParameterValue
TEST(TypeObjectUtilsTests, build_complete_annotation_parameter_inconsistent_data)
{
    AnnotationParameterValue bool_param = TypeObjectUtils::build_annotation_parameter_value(true);
    AnnotationParameterValue byte_param = TypeObjectUtils::build_annotation_parameter_value_byte(16);
    AnnotationParameterValue int8_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<int8_t>(16));
    AnnotationParameterValue uint8_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<uint8_t>(16));
    AnnotationParameterValue int16_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<int16_t>(16));
    AnnotationParameterValue uint16_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<uint16_t>(
                        16));
    AnnotationParameterValue int32_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<int32_t>(16));
    AnnotationParameterValue uint32_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<uint32_t>(
                        16));
    AnnotationParameterValue int64_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<int64_t>(16));
    AnnotationParameterValue uint64_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<uint64_t>(
                        16));
    AnnotationParameterValue float32_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<float>(16));
    AnnotationParameterValue float64_param = TypeObjectUtils::build_annotation_parameter_value(static_cast<double>(16));
    AnnotationParameterValue float128_param = TypeObjectUtils::build_annotation_parameter_value(
        static_cast<long double>(16));
    AnnotationParameterValue char8_param = TypeObjectUtils::build_annotation_parameter_value('A');
    AnnotationParameterValue char16_param = TypeObjectUtils::build_annotation_parameter_value(L'A');
    // TODO(jlbueno)
    AnnotationParameterValue enum_param = TypeObjectUtils::build_annotation_parameter_value_enum(
        static_cast<int32_t>(0));
    AnnotationParameterValue string8_param = TypeObjectUtils::build_annotation_parameter_value(
        eprosima::fastcdr::fixed_string<128>("Hello"));
    AnnotationParameterValue string16_param = TypeObjectUtils::build_annotation_parameter_value(std::wstring(L"Hello"));
    std::vector<AnnotationParameterValue> ann_param_seq;
    ann_param_seq.push_back(bool_param);
    ann_param_seq.push_back(byte_param);
    ann_param_seq.push_back(int8_param);
    ann_param_seq.push_back(uint8_param);
    ann_param_seq.push_back(int16_param);
    ann_param_seq.push_back(uint16_param);
    ann_param_seq.push_back(int32_param);
    ann_param_seq.push_back(uint32_param);
    ann_param_seq.push_back(int64_param);
    ann_param_seq.push_back(uint64_param);
    ann_param_seq.push_back(float32_param);
    ann_param_seq.push_back(float64_param);
    ann_param_seq.push_back(float128_param);
    ann_param_seq.push_back(char8_param);
    ann_param_seq.push_back(char16_param);
    ann_param_seq.push_back(enum_param);
    ann_param_seq.push_back(string8_param);
    ann_param_seq.push_back(string16_param);
    CommonAnnotationParameter common;
    std::vector<bool> expected_results = {false, false, false, false, false, false, false, false, false, false, false,
                                          false, false, false, false, false, false, false};
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    TypeIdentifier type_id;
    type_id._d(TK_BOOLEAN);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[0] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_BYTE);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[0] = false;
    expected_results[1] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_INT8);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[1] = false;
    expected_results[2] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_INT16);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[2] = false;
    expected_results[4] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_INT32);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[4] = false;
    expected_results[6] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_INT64);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[6] = false;
    expected_results[8] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_UINT8);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[8] = false;
    expected_results[3] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_UINT16);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[3] = false;
    expected_results[5] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_UINT32);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[5] = false;
    expected_results[7] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_UINT64);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[7] = false;
    expected_results[9] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_FLOAT32);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[9] = false;
    expected_results[10] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_FLOAT64);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[10] = false;
    expected_results[11] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_FLOAT128);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[11] = false;
    expected_results[12] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_CHAR8);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[12] = false;
    expected_results[13] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TK_CHAR16);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[13] = false;
    expected_results[14] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    small_string_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[14] = false;
    expected_results[16] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TI_STRING16_SMALL);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[16] = false;
    expected_results[17] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    large_string_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[17] = false;
    expected_results[16] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    type_id._d(TI_STRING16_LARGE);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[16] = false;
    expected_results[17] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    small_sequence_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    expected_results[17] = false;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    large_sequence_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    small_array_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    large_array_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    small_map_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    large_map_type_identifier(type_id);
    common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    register_basic_enum();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_enum", type_ids);
    common = TypeObjectUtils::build_common_annotation_parameter(0,
                    type_ids.type_identifier1());
    expected_results[15] = true;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
    register_basic_bitmask();
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_bitmask", type_ids);
    common = TypeObjectUtils::build_common_annotation_parameter(0,
                    type_ids.type_identifier1());
    expected_results[15] = false;
    check_annotation_parameter_consistency(common, ann_param_seq, expected_results);
}

// Build CompleteAnnotationParameter with empty name
TEST(TypeObjectUtilsTests, build_complete_annotation_parameter_empty_name)
{
    std::string parameter_name;
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonAnnotationParameter common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    int32_t param_value = 100;
    AnnotationParameterValue param = TypeObjectUtils::build_annotation_parameter_value(param_value);
    EXPECT_THROW(CompleteAnnotationParameter ann_param = TypeObjectUtils::build_complete_annotation_parameter(common,
            parameter_name, param), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteAnnotationParameter ann_param = TypeObjectUtils::build_complete_annotation_parameter(common,
            "parameter_name", param));
}

// Build CompleteAnnotationHeader with empty name
TEST(TypeObjectUtilsTests, build_complete_annotation_header_empty_name)
{
    std::string annotation_name;
    EXPECT_THROW(CompleteAnnotationHeader header = TypeObjectUtils::build_complete_annotation_header(annotation_name),
            InvalidArgumentError);
    EXPECT_NO_THROW(CompleteAnnotationHeader header = TypeObjectUtils::build_complete_annotation_header(
                "annotation_name"));
}

// Build CompleteAnnotationType with non-empty flags
TEST(TypeObjectUtilsTests, build_complete_annotation_type_non_empty_flags)
{
    AnnotationTypeFlag non_empty_flags = 1;
    AnnotationTypeFlag empty_flags = 0;
    CompleteAnnotationHeader header = TypeObjectUtils::build_complete_annotation_header("annotation_name");
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

// Build CompleteAliasBody with @hashid annotation applied
TEST(TypeObjectUtilsTests, build_complete_alias_body_inconsistent_hashid)
{
    TypeIdentifier type_id;
    type_id._d(TK_INT16);
    CommonAliasBody common_body = TypeObjectUtils::build_common_alias_body(0, type_id);
    AppliedBuiltinMemberAnnotations unit_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>("unit"), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>());
    // TODO(jlbueno) @min & @max annotations cannot be applied: TypeObject depends on 'any' block implementation.
    AppliedBuiltinMemberAnnotations hash_id_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("member_hash"));
    EXPECT_THROW(CompleteAliasBody body = TypeObjectUtils::build_complete_alias_body(common_body, hash_id_builtin_ann,
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteAliasBody body = TypeObjectUtils::build_complete_alias_body(common_body, unit_builtin_ann,
            eprosima::fastcdr::optional<AppliedAnnotationSeq>()));
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

// Build CompleteElementDetail with @hasid builtin annotation
TEST(TypeObjectUtilsTests, build_complete_element_detail_inconsistent_hashid)
{
    AppliedBuiltinMemberAnnotations unit_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>("unit"), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>());
    // TODO(jlbueno) @min & @max annotations cannot be applied: TypeObject depends on 'any' block implementation.
    AppliedBuiltinMemberAnnotations hash_id_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("member_hash"));
    EXPECT_THROW(CompleteElementDetail element_detail = TypeObjectUtils::build_complete_element_detail(
                hash_id_builtin_ann, eprosima::fastcdr::optional<AppliedAnnotationSeq>()), InvalidArgumentError);
    EXPECT_NO_THROW(CompleteElementDetail element_detail = TypeObjectUtils::build_complete_element_detail(
                unit_builtin_ann, eprosima::fastcdr::optional<AppliedAnnotationSeq>()));
}

// Build CompleteSequenceType with non-empty flags.
TEST(TypeObjectUtilsTests, build_complete_sequence_type_non_empty_flags)
{
    CollectionTypeFlag non_empty_flags = 1;
    CollectionTypeFlag empty_flags = 0;
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
                    eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
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
    LBound bound = 150;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    EXPECT_NO_THROW(CommonArrayHeader header = TypeObjectUtils::build_common_array_header(array_bounds));
    bound = 0;
    EXPECT_THROW(TypeObjectUtils::add_array_dimension(array_bounds, bound), InvalidArgumentError);
    array_bounds.push_back(bound);
    EXPECT_THROW(CommonArrayHeader header = TypeObjectUtils::build_common_array_header(array_bounds),
            InvalidArgumentError);
}

// Build CompleteArrayType with non-empty flags.
TEST(TypeObjectUtilsTests, build_complete_array_type_non_empty_flags)
{
    CollectionTypeFlag non_empty_flags = 1;
    CollectionTypeFlag empty_flags = 0;
    LBoundSeq array_bounds;
    LBound bound = 356;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    CommonArrayHeader common_header = TypeObjectUtils::build_common_array_header(array_bounds);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "array_type_name");
    CompleteArrayHeader header = TypeObjectUtils::build_complete_array_header(common_header, type_detail);
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
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
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
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
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
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
    key_type_id._d(TK_BOOLEAN);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    key_type_id._d(TK_BYTE);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    key_type_id._d(TK_FLOAT64);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    key_type_id._d(TK_FLOAT128);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    key_type_id._d(TK_CHAR8);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    key_type_id._d(TK_CHAR16);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    small_string_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element));
    key_type_id._d(TI_STRING16_SMALL);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element));
    large_string_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element));
    key_type_id._d(TI_STRING16_LARGE);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element));
    small_sequence_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    large_sequence_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    small_array_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    large_array_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    small_map_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    large_map_type_identifier(key_type_id);
    common_key = TypeObjectUtils::build_common_collection_element(flags, key_type_id);
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    register_basic_enum();
    TypeIdentifierPair type_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_enum", type_ids);
    common_key = TypeObjectUtils::build_common_collection_element(flags,
                    type_ids.type_identifier1());
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    register_basic_bitmask();
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic_bitmask", type_ids);
    common_key = TypeObjectUtils::build_common_collection_element(flags,
                    type_ids.type_identifier1());
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    CompleteAliasType int16_alias = int_16_alias();
    CompleteAliasType float_alias = float32_alias();
    TypeObjectUtils::build_and_register_alias_type_object(int16_alias, "alias_int16", type_ids);
    TypeObjectUtils::build_and_register_alias_type_object(float_alias, "alias_float", type_ids);
    TypeIdentifierPair alias_type_identifiers;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers("alias_float", alias_type_identifiers));
    common_key = TypeObjectUtils::build_common_collection_element(flags,
                    alias_type_identifiers.type_identifier1());
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element), InvalidArgumentError);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers("alias_int16", alias_type_identifiers));
    common_key = TypeObjectUtils::build_common_collection_element(flags,
                    alias_type_identifiers.type_identifier1());
    key = TypeObjectUtils::build_complete_collection_element(common_key, detail);
    EXPECT_NO_THROW(CompleteMapType map = TypeObjectUtils::build_complete_map_type(empty_flags, header,
            key, element));
}

// Build CompleteEnumeratedLiteral with non-applying builtin annotations.
TEST(TypeObjectUtilsTests, build_complete_enumerated_literal_invalid_builtin_annotations)
{
    AppliedBuiltinMemberAnnotations unit_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>("unit"), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>());
    // TODO(jlbueno) @min & @max annotations cannot be applied: TypeObject depends on 'any' block implementation.
    AppliedBuiltinMemberAnnotations hash_id_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("member_hash"));
    CompleteMemberDetail unit_member = TypeObjectUtils::build_complete_member_detail("member_name", unit_builtin_ann,
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail hash_id_member = TypeObjectUtils::build_complete_member_detail("member_name",
                    hash_id_builtin_ann, eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteEnumeratedLiteral enum_literal = TypeObjectUtils::build_complete_enumerated_literal(
                CommonEnumeratedLiteral(), unit_member), InvalidArgumentError);
    EXPECT_THROW(CompleteEnumeratedLiteral enum_literal = TypeObjectUtils::build_complete_enumerated_literal(
                CommonEnumeratedLiteral(), hash_id_member), InvalidArgumentError);
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

// Build CompleteBitflag with non-applying builtin annotations.
TEST(TypeObjectUtilsTests, build_complete_bitflag_invalid_builtin_annotations)
{
    AppliedBuiltinMemberAnnotations unit_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>("unit"), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>());
    // TODO(jlbueno) @min & @max annotations cannot be applied: TypeObject depends on 'any' block implementation.
    AppliedBuiltinMemberAnnotations hash_id_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("member_hash"));
    CompleteMemberDetail unit_member = TypeObjectUtils::build_complete_member_detail("member_name", unit_builtin_ann,
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail hash_id_member = TypeObjectUtils::build_complete_member_detail("member_name",
                    hash_id_builtin_ann, eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    EXPECT_THROW(CompleteBitflag bitflag = TypeObjectUtils::build_complete_bitflag(CommonBitflag(), unit_member),
            InvalidArgumentError);
    EXPECT_THROW(CompleteBitflag bitflag = TypeObjectUtils::build_complete_bitflag(CommonBitflag(), hash_id_member),
            InvalidArgumentError);
}

// Build CommonEnumeratedHeader with inconsistent bitmask bit bound
TEST(TypeObjectUtilsTests, build_common_enumerated_header_inconsistent_bitmask_bit_bound)
{
    BitBound bit_bound = 0;
    EXPECT_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound, true),
            InvalidArgumentError);
    bit_bound = 33;
    EXPECT_NO_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound, true));
    bit_bound = 65;
    EXPECT_THROW(CommonEnumeratedHeader header = TypeObjectUtils::build_common_enumerated_header(bit_bound, true),
            InvalidArgumentError);
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

// Build CompleteBitfield with non-applying builtin annotations.
TEST(TypeObjectUtilsTests, build_complete_bitfield_invalid_builtin_annotations)
{
    AppliedBuiltinMemberAnnotations unit_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>("unit"), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>());
    // TODO(jlbueno) @min & @max annotations cannot be applied: TypeObject depends on 'any' block implementation.
    AppliedBuiltinMemberAnnotations hash_id_builtin_ann = TypeObjectUtils::build_applied_builtin_member_annotations(
        eprosima::fastcdr::optional<std::string>(), eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<AnnotationParameterValue>(),
        eprosima::fastcdr::optional<std::string>("member_hash"));
    CompleteMemberDetail unit_member = TypeObjectUtils::build_complete_member_detail("member_name", unit_builtin_ann,
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail hash_id_member = TypeObjectUtils::build_complete_member_detail("member_name",
                    hash_id_builtin_ann, eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CommonBitfield common_bitfield = TypeObjectUtils::build_common_bitfield(0, 0, 1, TK_BOOLEAN);
    EXPECT_THROW(CompleteBitfield bitfield = TypeObjectUtils::build_complete_bitfield(common_bitfield, unit_member),
            InvalidArgumentError);
    EXPECT_THROW(CompleteBitfield bitfield = TypeObjectUtils::build_complete_bitfield(common_bitfield, hash_id_member),
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
    TypeIdentifierPair type_ids;
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
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_alias_type_object(alias, "alias", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_alias_type_object(alias, "alias", type_ids));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "alias_name");
    CompleteAliasHeader header = TypeObjectUtils::build_complete_alias_header(detail);
    CompleteAliasType other_alias = TypeObjectUtils::build_complete_alias_type(0, header, body);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_alias_type_object(other_alias, "alias", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_alias_type_object(
                other_alias,
                type_name,
                type_ids));
}

// Register annotation TypeObject
TEST(TypeObjectUtilsTests, register_annotation_type_object)
{
    CompleteAnnotationHeader header = TypeObjectUtils::build_complete_annotation_header("annotation_name");
    CompleteAnnotationType annotation = TypeObjectUtils::build_complete_annotation_type(0, header,
                    CompleteAnnotationParameterSeq());
    TypeIdentifierPair type_ids;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(annotation,
            "annotation", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_annotation_type_object(annotation,
            "annotation", type_ids));
    CompleteAnnotationHeader other_header = TypeObjectUtils::build_complete_annotation_header("other_annotation_name");
    CompleteAnnotationType other_annotation = TypeObjectUtils::build_complete_annotation_type(0, other_header,
                    CompleteAnnotationParameterSeq());
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_annotation_type_object(
                other_annotation, "annotation", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_annotation_type_object(
                other_annotation, type_name,
                type_ids));
}

// Register structure TypeObject
TEST(TypeObjectUtilsTests, register_structure_type_object)
{
    StructTypeFlag flags = TypeObjectUtils::build_struct_type_flag(
        eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE, false, false);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteStructHeader header = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), empty_type_detail);
    CompleteStructType structure = TypeObjectUtils::build_complete_struct_type(flags, header,
                    CompleteStructMemberSeq());
    TypeIdentifierPair type_ids;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_struct_type_object(structure,
            "structure", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_struct_type_object(structure,
            "structure", type_ids));
    StructTypeFlag other_flags = TypeObjectUtils::build_struct_type_flag(
        eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL, false, false);
    CompleteStructType other_structure = TypeObjectUtils::build_complete_struct_type(other_flags,
                    header, CompleteStructMemberSeq());
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_struct_type_object(
                other_structure, "structure", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_struct_type_object(
                other_structure, type_name,
                type_ids));
}

// Register union TypeObject
TEST(TypeObjectUtilsTests, register_union_type_object)
{
    UnionTypeFlag flags = TypeObjectUtils::build_union_type_flag(
        eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE, false, false);
    UnionDiscriminatorFlag discr_flags = TypeObjectUtils::build_union_discriminator_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
        false);
    TypeIdentifier discriminator_type_id;
    discriminator_type_id._d(TK_BYTE);
    CommonDiscriminatorMember discr_member = TypeObjectUtils::build_common_discriminator_member(discr_flags,
                    discriminator_type_id);
    CompleteDiscriminatorMember discriminator = TypeObjectUtils::build_complete_discriminator_member(discr_member,
                    eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    UnionMemberFlag member_flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    TypeIdentifierPair type_ids;
    UnionCaseLabelSeq case_labels;
    TypeObjectUtils::add_union_case_label(case_labels, 5);
    CommonUnionMember common_member = TypeObjectUtils::build_common_union_member(3, member_flags, type_id, case_labels);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember member = TypeObjectUtils::build_complete_union_member(common_member, empty_member_detail);
    CompleteUnionMemberSeq member_seq;
    TypeObjectUtils::add_complete_union_member(member_seq, member);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteUnionHeader header = TypeObjectUtils::build_complete_union_header(empty_type_detail);
    CompleteUnionType union_type = TypeObjectUtils::build_complete_union_type(flags, header,
                    discriminator, member_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_union_type_object(union_type, "union", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_union_type_object(union_type, "union", type_ids));
    UnionTypeFlag other_flags = TypeObjectUtils::build_union_type_flag(
        eprosima::fastdds::dds::xtypes::ExtensibilityKind::MUTABLE, false, false);
    CompleteUnionType other_union_type = TypeObjectUtils::build_complete_union_type(other_flags, header,
                    discriminator, member_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_union_type_object(
                other_union_type, "union", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_union_type_object(
                other_union_type, type_name,
                type_ids));
}

// Register bitset TypeObject
TEST(TypeObjectUtilsTests, register_bitset_type_object)
{
    TypeIdentifierPair type_ids;
    CommonBitfield common_bitfield = TypeObjectUtils::build_common_bitfield(0, 0, 3, TK_BYTE);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitfield bitfield = TypeObjectUtils::build_complete_bitfield(common_bitfield, empty_member_detail);
    CompleteBitfieldSeq bitfield_seq;
    TypeObjectUtils::add_complete_bitfield(bitfield_seq, bitfield);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteBitsetHeader header = TypeObjectUtils::build_complete_bitset_header(empty_type_detail);
    CompleteBitsetType bitset = TypeObjectUtils::build_complete_bitset_type(0, header, bitfield_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_bitset_type_object(bitset, "bitset", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_bitset_type_object(bitset, "bitset", type_ids));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "bitset");
    CompleteBitsetHeader other_header = TypeObjectUtils::build_complete_bitset_header(detail);
    CompleteBitsetType other_bitset = TypeObjectUtils::build_complete_bitset_type(0, other_header, bitfield_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_bitset_type_object(
                other_bitset, "bitset", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_bitset_type_object(
                other_bitset, type_name,
                type_ids));
}

// Register sequence TypeObject
TEST(TypeObjectUtilsTests, register_sequence_type_object)
{
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
                    eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
                    CompleteElementDetail());
    CompleteSequenceType sequence = TypeObjectUtils::build_complete_sequence_type(0, header, element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_sequence_type_object(sequence, "sequence"));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_sequence_type_object(sequence, "sequence"));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "sequence");
    CompleteCollectionHeader other_header = TypeObjectUtils::build_complete_collection_header(common_header, detail);
    CompleteSequenceType other_sequence = TypeObjectUtils::build_complete_sequence_type(0, other_header, element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, TypeObjectUtils::build_and_register_sequence_type_object(
                other_sequence, "sequence"));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_sequence_type_object(
                other_sequence,
                type_name));
}

// Register array TypeObject
TEST(TypeObjectUtilsTests, register_array_type_object)
{
    LBoundSeq array_bounds;
    LBound bound = 356;
    TypeObjectUtils::add_array_dimension(array_bounds, bound);
    CommonArrayHeader common_header = TypeObjectUtils::build_common_array_header(array_bounds);
    CompleteTypeDetail empty_type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteArrayHeader header = TypeObjectUtils::build_complete_array_header(common_header, empty_type_detail);
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_FLOAT128);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
                    CompleteElementDetail());
    CompleteArrayType array = TypeObjectUtils::build_complete_array_type(0, header, element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_array_type_object(array, "array"));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_array_type_object(array, "array"));
    type_id._d(TK_INT16);
    CommonCollectionElement other_common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement other_element = TypeObjectUtils::build_complete_collection_element(other_common_element,
                    CompleteElementDetail());
    CompleteArrayType other_array = TypeObjectUtils::build_complete_array_type(0, header, other_element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_array_type_object(other_array,
            "array"));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_array_type_object(
                other_array,
                type_name));
}

// Register map TypeObject
TEST(TypeObjectUtilsTests, register_map_type_object)
{
    CommonCollectionHeader common_header = TypeObjectUtils::build_common_collection_header(356);
    CompleteCollectionHeader header = TypeObjectUtils::build_complete_collection_header(common_header,
                    eprosima::fastcdr::optional<CompleteTypeDetail>());
    CollectionElementFlag flags = TypeObjectUtils::build_collection_element_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::USE_DEFAULT, true);
    TypeIdentifier type_id;
    type_id._d(TK_INT16);
    CommonCollectionElement common_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement element = TypeObjectUtils::build_complete_collection_element(common_element,
                    CompleteElementDetail());
    CompleteMapType map = TypeObjectUtils::build_complete_map_type(0, header, element, element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_map_type_object(map, "map"));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_map_type_object(map, "map"));
    type_id._d(TK_INT32);
    CommonCollectionElement key_element = TypeObjectUtils::build_common_collection_element(flags, type_id);
    CompleteCollectionElement key = TypeObjectUtils::build_complete_collection_element(key_element,
                    CompleteElementDetail());
    CompleteMapType other_map = TypeObjectUtils::build_complete_map_type(0, header, key, element);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_map_type_object(other_map,
            "map"));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            TypeObjectUtils::build_and_register_map_type_object(other_map,
            type_name));
}

// Register enumeration TypeObject
TEST(TypeObjectUtilsTests, register_enumerated_type_object)
{
    TypeIdentifierPair type_ids;
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
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_enumerated_type_object(enumeration, "enum", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_enumerated_type_object(enumeration, "enum", type_ids));
    CompleteTypeDetail detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(),
        "enumeration");
    CompleteEnumeratedHeader other_header = TypeObjectUtils::build_complete_enumerated_header(common_header, detail);
    CompleteEnumeratedType other_enumeration = TypeObjectUtils::build_complete_enumerated_type(0, other_header,
                    literal_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_enumerated_type_object(other_enumeration, "enum", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_enumerated_type_object(
                other_enumeration, type_name,
                type_ids));
}

// Register bitmask TypeObject
TEST(TypeObjectUtilsTests, register_bitmask_type_object)
{
    TypeIdentifierPair type_ids;
    CommonEnumeratedHeader common_header = TypeObjectUtils::build_common_enumerated_header(64, true);
    CompleteTypeDetail type_detail = TypeObjectUtils::build_complete_type_detail(
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>(),
        eprosima::fastcdr::optional<AppliedAnnotationSeq>(), "type_name");
    CompleteBitmaskHeader header = TypeObjectUtils::build_complete_enumerated_header(common_header, type_detail, true);
    CompleteMemberDetail empty_member_detail = TypeObjectUtils::build_complete_member_detail("member_name",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitflag bitflag = TypeObjectUtils::build_complete_bitflag(CommonBitflag(), empty_member_detail);
    CompleteBitflagSeq bitflag_seq;
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, bitflag);
    CompleteBitmaskType bitmask = TypeObjectUtils::build_complete_bitmask_type(0, header, bitflag_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            TypeObjectUtils::build_and_register_bitmask_type_object(bitmask, "bitmask", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_bitmask_type_object(bitmask, "bitmask", type_ids));
    CommonBitflag common = TypeObjectUtils::build_common_bitflag(1, 0);
    CompleteMemberDetail other_member_detail = TypeObjectUtils::build_complete_member_detail("other_member_name",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteBitflag other_bitflag = TypeObjectUtils::build_complete_bitflag(common, other_member_detail);
    TypeObjectUtils::add_complete_bitflag(bitflag_seq, other_bitflag);
    CompleteBitmaskType other_bitmask = TypeObjectUtils::build_complete_bitmask_type(0, header, bitflag_seq);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            TypeObjectUtils::build_and_register_bitmask_type_object(other_bitmask, "bitmask", type_ids));
    std::string type_name;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET, TypeObjectUtils::build_and_register_bitmask_type_object(
                other_bitmask, type_name,
                type_ids));
}

// This tests only check that Complete TypeObjects sequences are correctly ordered. TypeObjectUtils API only generates
// Complete TypeObjects. Minimal TypeObject sequence order is checked in Fast DDS-Gen.
// Test add element to AppliedAnnotationParameterSeq
TEST(TypeObjectUtilsTests, add_to_applied_annotation_parameter_seq)
{
    NameHash get_dependencies_hash = TypeObjectUtils::name_hash("getDependencies"); // 0x31FBAA_5
    NameHash color_hash = TypeObjectUtils::name_hash("color");                      // 0x70DDA5DF
    NameHash shapesize_hash = TypeObjectUtils::name_hash("shapesize");              // 0xDA907714
    EXPECT_TRUE(color_hash < shapesize_hash);
    AnnotationParameterValue param_value = TypeObjectUtils::build_annotation_parameter_value(true);
    AppliedAnnotationParameter get_dependencies_param = TypeObjectUtils::build_applied_annotation_parameter(
        get_dependencies_hash, param_value);
    AppliedAnnotationParameter color_param = TypeObjectUtils::build_applied_annotation_parameter(color_hash,
                    param_value);
    AppliedAnnotationParameter shapesize_param = TypeObjectUtils::build_applied_annotation_parameter(shapesize_hash,
                    param_value);
    AppliedAnnotationParameterSeq param_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, shapesize_param));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, shapesize_param), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, get_dependencies_param));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, get_dependencies_param),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, color_param));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation_parameter(param_seq, color_param), InvalidArgumentError);
    EXPECT_EQ(3, param_seq.size());
    EXPECT_EQ(get_dependencies_param, param_seq[0]);
    EXPECT_EQ(color_param, param_seq[1]);
    EXPECT_EQ(shapesize_param, param_seq[2]);
    EXPECT_TRUE(param_seq[0].paramname_hash() < param_seq[1].paramname_hash());
    EXPECT_TRUE(param_seq[1].paramname_hash() < param_seq[2].paramname_hash());
}

// Test add element to AppliedAnnotationSeq
TEST(TypeObjectUtilsTests, add_to_applied_annotation_seq)
{
    CompleteAnnotationHeader first_ann = TypeObjectUtils::build_complete_annotation_header("first");
    CompleteAnnotationHeader second_ann = TypeObjectUtils::build_complete_annotation_header("second");
    CompleteAnnotationHeader third_ann = TypeObjectUtils::build_complete_annotation_header("third");
    CompleteAnnotationType first_custom_annotation = TypeObjectUtils::build_complete_annotation_type(0, first_ann,
                    CompleteAnnotationParameterSeq());
    CompleteAnnotationType second_custom_annotation = TypeObjectUtils::build_complete_annotation_type(0, second_ann,
                    CompleteAnnotationParameterSeq());
    CompleteAnnotationType third_custom_annotation = TypeObjectUtils::build_complete_annotation_type(0, third_ann,
                    CompleteAnnotationParameterSeq());
    TypeIdentifierPair type_ids;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(
                first_custom_annotation, "first_custom", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(
                second_custom_annotation, "second_custom", type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, TypeObjectUtils::build_and_register_annotation_type_object(
                third_custom_annotation, "third_custom", type_ids));
    TypeIdentifierPair first_custom_annotation_ids;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("first_custom",
            first_custom_annotation_ids));
    TypeIdentifierPair second_custom_annotation_ids;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("second_custom",
            second_custom_annotation_ids));
    TypeIdentifierPair third_custom_annotation_ids;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("third_custom",
            third_custom_annotation_ids));
    // Ensure to use Complete TypeIdentifier. The test uses annotations that differs on the name which is not included
    // in the Minimal TypeObject.
    AppliedAnnotation first_annotation;
    AppliedAnnotation second_annotation;
    AppliedAnnotation third_annotation;
    if (EK_COMPLETE == first_custom_annotation_ids.type_identifier1()._d())
    {
        first_annotation = TypeObjectUtils::build_applied_annotation(
            first_custom_annotation_ids.type_identifier1(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    else
    {
        first_annotation = TypeObjectUtils::build_applied_annotation(
            first_custom_annotation_ids.type_identifier2(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    if (EK_COMPLETE == second_custom_annotation_ids.type_identifier1()._d())
    {
        second_annotation = TypeObjectUtils::build_applied_annotation(
            second_custom_annotation_ids.type_identifier1(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    else
    {
        second_annotation = TypeObjectUtils::build_applied_annotation(
            second_custom_annotation_ids.type_identifier2(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    if (EK_COMPLETE == third_custom_annotation_ids.type_identifier1()._d())
    {
        third_annotation = TypeObjectUtils::build_applied_annotation(
            third_custom_annotation_ids.type_identifier1(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    else
    {
        third_annotation = TypeObjectUtils::build_applied_annotation(
            third_custom_annotation_ids.type_identifier2(),
            eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>());
    }
    AppliedAnnotationSeq applied_annotation_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, third_annotation));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, third_annotation),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, first_annotation));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, first_annotation),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, second_annotation));
    EXPECT_THROW(TypeObjectUtils::add_applied_annotation(applied_annotation_seq, second_annotation),
            InvalidArgumentError);
    EXPECT_EQ(3, applied_annotation_seq.size());
    // Ordered by Annotation TypeIdentifier
    EXPECT_TRUE(applied_annotation_seq[0].annotation_typeid().equivalence_hash() <
            applied_annotation_seq[1].annotation_typeid().equivalence_hash());
    EXPECT_TRUE(applied_annotation_seq[1].annotation_typeid().equivalence_hash() <
            applied_annotation_seq[2].annotation_typeid().equivalence_hash());
}

// Test add element to CompleteStructMemberSeq
TEST(TypeObjectUtilsTests, add_to_complete_struct_member_seq)
{
    StructMemberFlag flags = TypeObjectUtils::build_struct_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false, false,
        false);
    TypeIdentifier type_id;
    type_id._d(TK_INT32);
    CommonStructMember first_member = TypeObjectUtils::build_common_struct_member(0, flags, type_id);
    CommonStructMember second_member = TypeObjectUtils::build_common_struct_member(1, flags, type_id);
    CommonStructMember third_member = TypeObjectUtils::build_common_struct_member(2, flags, type_id);
    CompleteMemberDetail first_detail = TypeObjectUtils::build_complete_member_detail("first",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail second_detail = TypeObjectUtils::build_complete_member_detail("second",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail third_detail = TypeObjectUtils::build_complete_member_detail("third",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteStructMember first = TypeObjectUtils::build_complete_struct_member(first_member, first_detail);
    CompleteStructMember second = TypeObjectUtils::build_complete_struct_member(second_member, second_detail);
    CompleteStructMember third = TypeObjectUtils::build_complete_struct_member(third_member, third_detail);
    CompleteStructMember invalid = TypeObjectUtils::build_complete_struct_member(first_member, third_detail);
    CompleteStructMemberSeq member_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, third));
    EXPECT_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, third), InvalidArgumentError);
#if !defined(NDEBUG)
    EXPECT_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, invalid), InvalidArgumentError);
#endif // !defined(NDEBUG)
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, first));
    EXPECT_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, first),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, second));
    EXPECT_THROW(TypeObjectUtils::add_complete_struct_member(member_seq, second), InvalidArgumentError);
    EXPECT_EQ(3, member_seq.size());
    EXPECT_EQ(third, member_seq[0]);
    EXPECT_EQ(first, member_seq[1]);
    EXPECT_EQ(second, member_seq[2]);
}

// Test add element to UnionCaseLabelSeq
TEST(TypeObjectUtilsTests, add_to_union_case_label_seq)
{
    UnionCaseLabelSeq labels;
    EXPECT_NO_THROW(TypeObjectUtils::add_union_case_label(labels, 3));
    EXPECT_EQ(1, labels.size());
    EXPECT_NO_THROW(TypeObjectUtils::add_union_case_label(labels, 3));
    EXPECT_EQ(1, labels.size());
    EXPECT_NO_THROW(TypeObjectUtils::add_union_case_label(labels, 1));
    EXPECT_NO_THROW(TypeObjectUtils::add_union_case_label(labels, 2));
    EXPECT_EQ(3, labels.size());
    EXPECT_EQ(1, labels[0]);
    EXPECT_EQ(2, labels[1]);
    EXPECT_EQ(3, labels[2]);
}

// Test add element to CompleteUnionMemberSeq
TEST(TypeObjectUtilsTests, add_to_complete_union_member_seq)
{
    UnionMemberFlag flags = TypeObjectUtils::build_union_member_flag(
        eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD, false, false);
    TypeIdentifier type_id;
    type_id._d(TK_UINT32);
    UnionCaseLabelSeq label_1;
    UnionCaseLabelSeq label_2;
    UnionCaseLabelSeq label_3;
    TypeObjectUtils::add_union_case_label(label_1, 1);
    TypeObjectUtils::add_union_case_label(label_2, 2);
    TypeObjectUtils::add_union_case_label(label_3, 3);
    CommonUnionMember member_1 = TypeObjectUtils::build_common_union_member(1, flags, type_id, label_1);
    CommonUnionMember member_2 = TypeObjectUtils::build_common_union_member(2, flags, type_id, label_2);
    CommonUnionMember member_3 = TypeObjectUtils::build_common_union_member(3, flags, type_id, label_3);
    CompleteMemberDetail first_detail = TypeObjectUtils::build_complete_member_detail("first",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail second_detail = TypeObjectUtils::build_complete_member_detail("second",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail third_detail = TypeObjectUtils::build_complete_member_detail("third",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteUnionMember first_member = TypeObjectUtils::build_complete_union_member(member_1, first_detail);
    CompleteUnionMember second_member = TypeObjectUtils::build_complete_union_member(member_2, second_detail);
    CompleteUnionMember third_member = TypeObjectUtils::build_complete_union_member(member_3, third_detail);
    CompleteUnionMemberSeq member_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_union_member(member_seq, third_member));
    EXPECT_THROW(TypeObjectUtils::add_complete_union_member(member_seq, third_member), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_union_member(member_seq, first_member));
    EXPECT_THROW(TypeObjectUtils::add_complete_union_member(member_seq, first_member),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_union_member(member_seq, second_member));
    EXPECT_THROW(TypeObjectUtils::add_complete_union_member(member_seq, second_member), InvalidArgumentError);
    EXPECT_EQ(3, member_seq.size());
    EXPECT_EQ(third_member, member_seq[0]);
    EXPECT_EQ(first_member, member_seq[1]);
    EXPECT_EQ(second_member, member_seq[2]);
}

// Test add element to CompleteAnnotationParameterSeq
TEST(TypeObjectUtilsTests, add_to_complete_annotation_parameter_seq)
{
    TypeIdentifier type_id;
    type_id._d(TK_BOOLEAN);
    CommonAnnotationParameter common = TypeObjectUtils::build_common_annotation_parameter(0, type_id);
    AnnotationParameterValue default_value = TypeObjectUtils::build_annotation_parameter_value(false);
    MemberName first = "first";
    MemberName second = "second";
    MemberName third = "third";
    CompleteAnnotationParameter first_param = TypeObjectUtils::build_complete_annotation_parameter(common, first,
                    default_value);
    CompleteAnnotationParameter second_param = TypeObjectUtils::build_complete_annotation_parameter(common, second,
                    default_value);
    CompleteAnnotationParameter third_param = TypeObjectUtils::build_complete_annotation_parameter(common, third,
                    default_value);
    CompleteAnnotationParameterSeq param_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, third_param));
    EXPECT_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, third_param), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, first_param));
    EXPECT_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, first_param),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, second_param));
    EXPECT_THROW(TypeObjectUtils::add_complete_annotation_parameter(param_seq, second_param), InvalidArgumentError);
    EXPECT_EQ(3, param_seq.size());
    EXPECT_EQ(first_param, param_seq[0]);
    EXPECT_EQ(second_param, param_seq[1]);
    EXPECT_EQ(third_param, param_seq[2]);
}

// Test add element to CompleteEnumeratedLiteralSeq
TEST(TypeObjectUtils, add_to_complete_enumerated_literal_seq)
{
    EnumeratedLiteralFlag flags = TypeObjectUtils::build_enumerated_literal_flag(false);
    CommonEnumeratedLiteral first = TypeObjectUtils::build_common_enumerated_literal(1, flags);
    CommonEnumeratedLiteral second = TypeObjectUtils::build_common_enumerated_literal(2, flags);
    CommonEnumeratedLiteral third = TypeObjectUtils::build_common_enumerated_literal(3, flags);
    CompleteMemberDetail first_detail = TypeObjectUtils::build_complete_member_detail("first",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail second_detail = TypeObjectUtils::build_complete_member_detail("second",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail third_detail = TypeObjectUtils::build_complete_member_detail("third",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteEnumeratedLiteral first_literal = TypeObjectUtils::build_complete_enumerated_literal(first, first_detail);
    CompleteEnumeratedLiteral second_literal =
            TypeObjectUtils::build_complete_enumerated_literal(second, second_detail);
    CompleteEnumeratedLiteral third_literal = TypeObjectUtils::build_complete_enumerated_literal(third, third_detail);
    CompleteEnumeratedLiteralSeq literal_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, third_literal));
    EXPECT_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, third_literal), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, first_literal));
    EXPECT_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, first_literal),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, second_literal));
    EXPECT_THROW(TypeObjectUtils::add_complete_enumerated_literal(literal_seq, second_literal), InvalidArgumentError);
    EXPECT_EQ(3, literal_seq.size());
    EXPECT_EQ(first_literal, literal_seq[0]);
    EXPECT_EQ(second_literal, literal_seq[1]);
    EXPECT_EQ(third_literal, literal_seq[2]);
}

// Test add element to CompleteBitflagSeq
TEST(TypeObjectUtilsTests, add_to_complete_bitflag_seq)
{
    CompleteMemberDetail first_detail = TypeObjectUtils::build_complete_member_detail("first",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail second_detail = TypeObjectUtils::build_complete_member_detail("second",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail third_detail = TypeObjectUtils::build_complete_member_detail("third",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CommonBitflag first = TypeObjectUtils::build_common_bitflag(1, 0);
    CommonBitflag second = TypeObjectUtils::build_common_bitflag(2, 0);
    CommonBitflag third = TypeObjectUtils::build_common_bitflag(3, 0);
    CompleteBitflag first_bitflag = TypeObjectUtils::build_complete_bitflag(first, first_detail);
    CompleteBitflag second_bitflag = TypeObjectUtils::build_complete_bitflag(second, second_detail);
    CompleteBitflag third_bitflag = TypeObjectUtils::build_complete_bitflag(third, third_detail);
    CompleteBitflagSeq bitflag_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, third_bitflag));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, third_bitflag), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, first_bitflag));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, first_bitflag),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, second_bitflag));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitflag(bitflag_seq, second_bitflag), InvalidArgumentError);
    EXPECT_EQ(3, bitflag_seq.size());
    EXPECT_EQ(first_bitflag, bitflag_seq[0]);
    EXPECT_EQ(second_bitflag, bitflag_seq[1]);
    EXPECT_EQ(third_bitflag, bitflag_seq[2]);
}

// Test add element to CompleteBitfieldSeq
TEST(TypeObjectUtilsTests, add_to_complete_bitfield_seq)
{
    CompleteMemberDetail first_detail = TypeObjectUtils::build_complete_member_detail("first",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail second_detail = TypeObjectUtils::build_complete_member_detail("second",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CompleteMemberDetail third_detail = TypeObjectUtils::build_complete_member_detail("third",
                    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                    eprosima::fastcdr::optional<AppliedAnnotationSeq>());
    CommonBitfield first = TypeObjectUtils::build_common_bitfield(1, 0, 1, TK_BOOLEAN);
    CommonBitfield second = TypeObjectUtils::build_common_bitfield(2, 0, 1, TK_BOOLEAN);
    CommonBitfield third = TypeObjectUtils::build_common_bitfield(3, 0, 1, TK_BOOLEAN);
    CompleteBitfield first_bitfield = TypeObjectUtils::build_complete_bitfield(first, first_detail);
    CompleteBitfield second_bitfield = TypeObjectUtils::build_complete_bitfield(second, second_detail);
    CompleteBitfield third_bitfield = TypeObjectUtils::build_complete_bitfield(third, third_detail);
    CompleteBitfieldSeq bitfield_seq;
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, third_bitfield));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, third_bitfield), InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, first_bitfield));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, first_bitfield),
            InvalidArgumentError);
    EXPECT_NO_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, second_bitfield));
    EXPECT_THROW(TypeObjectUtils::add_complete_bitfield(bitfield_seq, second_bitfield), InvalidArgumentError);
    EXPECT_EQ(3, bitfield_seq.size());
    EXPECT_EQ(first_bitfield, bitfield_seq[0]);
    EXPECT_EQ(second_bitfield, bitfield_seq[1]);
    EXPECT_EQ(third_bitfield, bitfield_seq[2]);
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
    int ret_value = RUN_ALL_TESTS();
    eprosima::fastdds::dds::Log::KillThread();
    return ret_value;
}
