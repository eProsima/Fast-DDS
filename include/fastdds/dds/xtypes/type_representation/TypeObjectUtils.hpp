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

/*!
 * @file
 * This file contains static functions to help build a TypeObject.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_

#include <string>

#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

class TypeObjectUtils
{
public:

    /**
     * @brief Build TypeObjectHashId instance.
     *
     * @param[in] discriminator TypeObjectHashId discriminator to be set.
     * @param[in] hash StronglyConnectedComponent equivalence hash to be set.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given discriminator is not
     *            EK_COMPLETE/EK_MINIMAL.
     * @return const TypeObjectHashId instance.
     */
    RTPS_DllAPI static const TypeObjectHashId build_type_object_hash_id(
            uint8_t discriminator,
            const EquivalenceHash& hash);

    /**
     * @brief Build CollectionElementFlag instance.
     *
     * @param[in] try_construct_kind try_construct annotation value.
     * @param[in] external external annotation value.
     * @return CollectionElementFlag instance. 
     */
    RTPS_DllAPI static CollectionElementFlag build_collection_element_flag(
            TryConstructKind try_construct_kind,
            bool external);

    /**
     * @brief Build StructMemberFlag instance.
     *
     * @param[in] try_construct_kind try_construct annotation value.
     * @param[in] optional optional annotation value.
     * @param[in] must_understand must_understand annotation value.
     * @param[in] key key annotation value.
     * @param[in] external external annotation value.
     * @return StructMemberFlag instance.
     */
    RTPS_DllAPI static StructMemberFlag build_struct_member_flag(
            TryConstructKind try_construct_kind,
            bool optional,
            bool must_understand,
            bool key,
            bool external);

    /**
     * @brief Build UnionMemberFlag instance.
     *
     * @param[in] try_construct_kind try_construct annotation value.
     * @param[in] default_member is default member.
     * @param[in] external external annotation value.
     * @return UnionMemberFlag instance. 
     */
    RTPS_DllAPI static UnionMemberFlag build_union_member_flag(
            TryConstructKind try_construct_kind,
            bool default_member,
            bool external);

    /**
     * @brief Build UnionDiscriminatorFlag instance.
     *
     * @param[in] try_construct_kind try_construct annotation value.
     * @param[in] key key annotation value.
     * @return UnionDiscriminatorFlag instance.
     */
    RTPS_DllAPI static UnionDiscriminatorFlag build_union_discriminator_flag(
            TryConstructKind try_construct_kind,
            bool key);

    /**
     * @brief Build EnumeratedLiteralFlag instance.
     *
     * @param[in] default_literal default_literal annotation value.
     * @return EnumeratedLiteralFlag instance.
     */
    RTPS_DllAPI static EnumeratedLiteralFlag build_enumerated_literal_flag(
            bool default_literal);

    /**
     * AnnotationParameterFlag: Unused. No flags apply. 
     * AliasMemberFlag:         Unused. No flags apply.
     * BitflagFlag:             Unused. No flags apply.
     * BitsetMemberFlag:        Unused. No flags apply.
     */

    /**
     * @brief Build StructTypeFlag instance.
     *
     * @param[in] extensibility_kind extensibility annotation value.
     * @param[in] nested nested annotation value.
     * @param[in] autoid_hash autoid annotation has HASH value.
     * @return StructTypeFlag instance.
     */
    RTPS_DllAPI static StructTypeFlag build_struct_type_flag(
            ExtensibilityKind extensibility_kind,
            bool nested,
            bool autoid_hash);

    /**
     * @brief Build UnionTypeFlag instance.
     *
     * @param extensibility_kind extensibility annotation value.
     * @param nested nested annotation value.
     * @param autoid_hash autoid annotation has HASH value.
     * @return UnionTypeFlag instance. 
     */
    RTPS_DllAPI static UnionTypeFlag build_union_type_flag(
            ExtensibilityKind extensibility_kind,
            bool nested,
            bool autoid_hash);

    /**
     * CollectionTypeFlag:  Unused. No flags apply.
     * AnnotationTypeFlag:  Unused. No flags apply.
     * AliasTypeFlag:       Unused. No flags apply.
     * EnumTypeFlag:        Unused. No flags apply.
     * BitmaskTypeFlag:     Unused. No flags apply.
     * BitsetTypeFlag:      Unused. No flags apply.
     */

    /*************** Indirect Hash TypeIdentifiers ***************************/

    /**
     * @brief Build StringSTypeDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @param[in] bound Bound for the small string/wstring.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if bound is 0.
     * @return const StringSTypeDefn instance.
     */
    RTPS_DllAPI static const StringSTypeDefn build_string_s_type_defn(
            SBound bound);

    /**
     * @brief Build StringLTypeDefn instance.
     *
     * @pre bound > 255
     * @param[in] bound Bound for the large string/wstring.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if bound is lower than 256.
     * @return const StringLTypeDefn instance.
     */
    RTPS_DllAPI static const StringLTypeDefn build_string_l_type_defn(
            LBound bound);

    /**
     * @brief Build PlainCollectionHeader instance.
     *
     * @param equiv_kind EquivalenceKind: EK_MINIMAL/EK_COMPLETE/EK_BOTH
     * @param element_flags CollectionElementFlags to be set. This element must be constructed with the corresponding
     *                      builder to ensure its consistency.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError if the given element_flags are inconsistent.
     *            This exception is only thrown in Debug build mode.
     * @return const PlainCollectionHeader instance.
     */
    RTPS_DllAPI static const PlainCollectionHeader build_plain_collection_header(
            EquivalenceKindValue equiv_kind,
            CollectionElementFlag element_flags);

    /**
     * @brief Build PlainSequenceSElemDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param bound Sequence bound.
     * @param element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. The given bound is 0.
     *              2. Non-initialized TypeIdentifier (TK_NONE).
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainSequenceSElemDefn instance.
     */
    RTPS_DllAPI static const PlainSequenceSElemDefn build_plain_sequence_s_elem_defn(
            const PlainCollectionHeader& header,
            SBound s_bound,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Build PlainSequenceLElemDefn instance.
     *
     * @pre bound > 255
     * @pre element_identifier has been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param bound Sequence bound.
     * @param element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Bound lower than 256.
     *              2. Non-initialized TypeIdentifier (TK_NONE).
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainSequenceLElemDefn instance.
     */
    RTPS_DllAPI static const PlainSequenceLElemDefn build_plain_sequence_l_elem_defn(
            const PlainCollectionHeader& header,
            LBound bound,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Add dimension bound to the array bound sequence.
     *
     * @tparam array Either a SBoundSeq or LBoundSeq.
     * @tparam element Either a SBound or LBound.
     * @param[in out] array_bound_seq Sequence with the array bounds.
     * @param[in] dimension_bound Dimension bound to be added into the sequence.
     */
    template<typename array, typename element>
    RTPS_DllAPI static void add_array_dimension(
            array& array_bound_seq,
            element dimension_bound)
    {
        array_bound_seq.push_back(dimension_bound);
    }

    /**
     * @brief Build PlainArraySElemDefn instance.
     *
     * @pre Any element in array_bound_seq must be greater than 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param array_bound_seq Bounds for the array dimensions.
     * @param element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. Non-initialized TypeIdentifier (TK_NONE).
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainArraySElemDefn instance.
     */
    RTPS_DllAPI static const PlainArraySElemDefn build_plain_array_s_elem_defn(
            const PlainCollectionHeader& header,
            const SBoundSeq& array_bound_seq,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Build PlainArrayLElemDefn instance.
     *
     * @pre At least one element of array_bound_seq must be greater than 255 and no element must be 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param array_bound_seq Bounds for the array dimensions.
     * @param element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. There is no dimension with a bound greater than 255.
     *              2. Non-initialized TypeIdentifier (TK_NONE).
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainArrayLElemDefn instance.
     */
    RTPS_DllAPI static const PlainArrayLElemDefn build_plain_array_l_elem_defn(
            const PlainCollectionHeader& header,
            const LBoundSeq& array_bound_seq,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Build PlainMapSTypeDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @pre Both element_identifier and key_identifier have been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param bound Map bound.
     * @param element_identifier Map element TypeIdentifier.
     * @param key_flags Flags applying to map key.
     * @param key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Given bound is zero (INVALID_SBOUND)
     *              2. Any given TypeIdentifier is not initialized (TK_NONE)
     *              3. Inconsistent element_identifier EquivalenceKind with the one contained in the header.
     *              4. Direct hash key_identifier or indirect hash TypeIdentifier with exception to string/wstring.
     *                 XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements
     *                 of signed and unsigned integer types and of narrow and wide string types.
     *              5. Inconsistent key_flags.
     *              6. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainMapSTypeDefn instance.
     */
    RTPS_DllAPI static const PlainMapSTypeDefn build_plain_map_s_type_defn(
            const PlainCollectionHeader& header,
            const SBound bound,
            const TypeIdentifier& element_identifier,
            const CollectionElementFlag key_flags,
            const TypeIdentifier& key_identifier);

    /**
     * @brief Build PlainMapLTypeDefn instance.
     *
     * @pre bound > 255
     * @pre Both element_identifier and key_identifier have been initialized.
     * @param header PlainCollectionHeader to be set.
     * @param bound Map bound.
     * @param element_identifier Map element TypeIdentifier.
     * @param key_flags Flags applying to map key.
     * @param key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Given bound is lower than 256
     *              2. Any given TypeIdentifier is not initialized (TK_NONE)
     *              3. Inconsistent element_identifier EquivalenceKind with the one contained in the header.
     *              4. Direct hash key_identifier or indirect hash TypeIdentifier with exception to string/wstring.
     *                 XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements
     *                 of signed and unsigned integer types and of narrow and wide string types.
     *              5. Inconsistent key_flags.
     *              6. Inconsistent PlainCollectionHeader (only in Debug build mode).
     * @return const PlainMapLTypeDefn instance.
     */
    RTPS_DllAPI static const PlainMapLTypeDefn build_plain_map_l_type_defn(
            const PlainCollectionHeader& header,
            const LBound bound,
            const TypeIdentifier& element_identifier,
            const CollectionElementFlag key_flags,
            const TypeIdentifier& key_identifier);

private:

    // Class with only static methods
    TypeObjectUtils() = delete;
    ~TypeObjectUtils() = delete;

protected:

    // Auxiliary methods
    /**
     * @brief Set the try construct behavior in a given MemberFlag
     *
     * @param[in out] member_flag Bitmask to be set.
     * @param[in] try_construct_kind TryConstructKind.
     */
    static void set_try_construct_behavior(
            MemberFlag& member_flag,
            TryConstructKind try_construct_kind);

    /**
     * @brief Set the TypeFlag object.
     * 
     * @param[in out] type_flag Bitmask to be set.
     * @param[in] extensibility_kind ExtensibilityKind
     * @param[in] nested nested annotation value.
     * @param[in] autoid_hash autoid annotation has HASH value.
     */
    static void set_type_flag(
            TypeFlag& type_flag,
            ExtensibilityKind extensibility_kind,
            bool nested,
            bool autoid_hash);

    /**
     * @brief Set the extensibility kind in a given TypeFlag.
     * 
     * @param type_flag Bitmask to be set.
     * @param extensibility_kind ExtensibilityKind.
     */
    static void set_extensibility_kind(
            TypeFlag& type_flag,
            ExtensibilityKind extensibility_kind);

    /**
     * @brief Check if a given TypeIdentifier is fully-descriptive.
     *        XTypes v1.3 Clause 7.3.4.6.1 Fully-descriptive TypeIdentifiers
     *        Some TypeIdentifiers do not involve computing the Hash of any TypeObject. These are called
     *        Fully-descriptive TypeIdentifiers because they fully describe the Type. These are:
     *        - The TypeIdentifiers for Primitive and String types.
     *        - The TypeIdentifiers of plain collections where the element (and key) TypeIdentifier is a fully
     *          descriptive TypeIdentifier. They are recognized by the contained PlainCollectionHeader having
     *          EquivalenceKind set to EK_BOTH.
     *
     * @param type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is fully-descriptive. false otherwise.
     */
    static bool is_fully_descriptive_type_identifier(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check if a given TypeIdentifier is direct hash.
     *        XTypes v1.3 Clause 7.3.4.6.3
     *        These are HASH TypeIdentifiers with discriminator EK_MINIMAL, EK_COMPLETE or TI_STRONG_COMPONENT.
     *
     * @param type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is direct hash. false otherwise.
     */
    static bool is_direct_hash_type_identifier(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check if a given TypeIdentifier is indirect hash.
     *        XTypes v1.3 Clause 7.3.4.6.4
     *        These are the TypeIdentifiers for plain collections with the element type identified using a HASH
     *        TypeIdentifier. They are distinguished by:
     *        1. Having discriminator TI_PLAIN_SEQUENCE_SMALL, TI_PLAIN_SEQUENCE_LARGE, TI_PLAIN_ARRAY_SMALL,
     *           TI_PLAIN_ARRAY_LARGE, TI_PLAIN_MAP_SMALL, or TI_PLAIN_MAP_LARGE.
     *        2. Having the contained PlainCollectionHeader with EquivalenceKind EK_MINIMAL or EK_COMPLETE.
     *
     * @param type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is indirect hash. false otherwise.
     */
    static bool is_indirect_hash_type_identifier(
            const TypeIdentifier& type_identifier);

    /*************** Consistent methods (Debug) ***************************/

    /**
     * @brief Check SBound consistency: must be different from 0.
     *
     * @param bound SBound to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given SBound is not
     *            consistent.
     */
    static void s_bound_consistency(
            SBound bound);

    /**
     * @brief Check LBound consistency: must be greater than 255.
     *
     * @param bound LBound to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given LBound is not
     *            consistent.
     */
    static void l_bound_consistency(
            LBound bound);

    /**
     * @brief Check that the array_bound_seq is consistent: non-empty.
     *
     * @tparam T Either SBoundSeq or LBoundSeq
     * @param array Sequence to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given array is not
     *            consistent.
     */
    template<typename T>
    static void array_bound_seq_consistency(
            const T& array)
    {
        if (array.empty())
        {
            throw InvalidArgumentError("array_bound_seq parameter must not be empty");
        }
    }

#if !defined(NDEBUG)
    /**
     * @brief Check MemberFlag consistency: At least one of the bits corresponding to the try construct annotation must
     *        be set.
     * 
     * @param member_flags MemberFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given MemberFlag is not
     *            consistent.
     */
    static void member_flag_consistency(
            MemberFlag member_flags);

    /**
     * @brief Check PlainCollectionHeader consistency:
     *          - CollectionElementFlag consistent
     *          - Consistent EquivalenceKind
     *
     * @param header PlainCollectionHeader to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainCollectionHeader
     *            is not consistent.
     */
    static void plain_collection_header_consistency(
            const PlainCollectionHeader& header);
#endif // !defined(NDEBUG)

    /**
     * @brief Check consistency between a given PlainCollectionHeader and the related TypeIdentifier:
     *        1. TypeIdentifier initialized
     *        2. Consistency of EquivalenceKinds
     *
     * @param header PlainCollectionHeader to be checked.
     * @param element_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given parameters are not
     *            consistent.
     */
    static void plain_collection_type_identifier_header_consistency(
            const PlainCollectionHeader& header,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Check map key_identifier consistency.
     *        XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements of signed
     *        and unsigned integer types and of narrow and wide string types.
     *        In Debug mode, this method also checks that the string/wstring bound is consistent.
     *
     * @param key_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeIdentifier is not
     *            consistent.
     */
    static void map_key_type_identifier_consistency(
            const TypeIdentifier& key_identifier);

};

} // xtypes1_3
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_
