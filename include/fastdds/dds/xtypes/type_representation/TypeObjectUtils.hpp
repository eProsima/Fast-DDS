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

#ifndef FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP
#define FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP

#include <string>

#include <fastcdr/cdr/fixed_size_string.hpp>
#include <fastcdr/xcdr/optional.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeBuilderFactoryImpl;

namespace xtypes {

class TypeObjectRegistry;

using ReturnCode_t = eprosima::fastdds::dds::ReturnCode_t;

class TypeObjectUtils
{
public:

    /**
     * @brief Build TypeObjectHashId instance.
     *
     * @param [in] discriminator TypeObjectHashId discriminator to be set.
     * @param [in] hash StronglyConnectedComponent equivalence hash to be set.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given discriminator is not
     *            EK_COMPLETE/EK_MINIMAL.
     * @return const TypeObjectHashId instance.
     */
    FASTDDS_EXPORTED_API static const TypeObjectHashId build_type_object_hash_id(
            uint8_t discriminator,
            const EquivalenceHash& hash);

    /**
     * @brief Build CollectionElementFlag instance.
     *
     * @param [in] try_construct_kind try_construct annotation value.
     * @param [in] external external annotation value.
     * @return CollectionElementFlag instance.
     */
    FASTDDS_EXPORTED_API static CollectionElementFlag build_collection_element_flag(
            TryConstructFailAction try_construct_kind,
            bool external);

    /**
     * @brief Build StructMemberFlag instance.
     *
     * @param [in] try_construct_kind try_construct annotation value.
     * @param [in] optional optional annotation value.
     * @param [in] must_understand must_understand annotation value.
     * @param [in] key key annotation value.
     * @param [in] external external annotation value.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if both key and optional flags are
     *            enabled.
     * @return StructMemberFlag instance.
     */
    FASTDDS_EXPORTED_API static StructMemberFlag build_struct_member_flag(
            TryConstructFailAction try_construct_kind,
            bool optional,
            bool must_understand,
            bool key,
            bool external);

    /**
     * @brief Build UnionMemberFlag instance.
     *
     * @param [in] try_construct_kind try_construct annotation value.
     * @param [in] default_member is default member.
     * @param [in] external external annotation value.
     * @return UnionMemberFlag instance.
     */
    FASTDDS_EXPORTED_API static UnionMemberFlag build_union_member_flag(
            TryConstructFailAction try_construct_kind,
            bool default_member,
            bool external);

    /**
     * @brief Build UnionDiscriminatorFlag instance.
     *
     * @param [in] try_construct_kind try_construct annotation value.
     * @param [in] key key annotation value.
     * @return UnionDiscriminatorFlag instance.
     */
    FASTDDS_EXPORTED_API static UnionDiscriminatorFlag build_union_discriminator_flag(
            TryConstructFailAction try_construct_kind,
            bool key);

    /**
     * @brief Build EnumeratedLiteralFlag instance.
     *
     * @param [in] default_literal default_literal annotation value.
     * @return EnumeratedLiteralFlag instance.
     */
    FASTDDS_EXPORTED_API static EnumeratedLiteralFlag build_enumerated_literal_flag(
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
     * @param [in] extensibility_kind extensibility annotation value.
     * @param [in] nested nested annotation value.
     * @param [in] autoid_hash autoid annotation has HASH value.
     * @return StructTypeFlag instance.
     */
    FASTDDS_EXPORTED_API static StructTypeFlag build_struct_type_flag(
            ExtensibilityKind extensibility_kind,
            bool nested,
            bool autoid_hash);

    /**
     * @brief Build UnionTypeFlag instance.
     *
     * @param [in] extensibility_kind extensibility annotation value.
     * @param [in] nested nested annotation value.
     * @param [in] autoid_hash autoid annotation has HASH value.
     * @return UnionTypeFlag instance.
     */
    FASTDDS_EXPORTED_API static UnionTypeFlag build_union_type_flag(
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

    //{{{ Indirect Hash TypeIdentifiers

    /**
     * @brief Build StringSTypeDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @param [in] bound Bound for the small string/wstring.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if bound is 0.
     * @return const StringSTypeDefn instance.
     */
    FASTDDS_EXPORTED_API static const StringSTypeDefn build_string_s_type_defn(
            SBound bound);

    /**
     * @brief Build StringLTypeDefn instance.
     *
     * @pre bound > 255
     * @param [in] bound Bound for the large string/wstring.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if bound is lower than 256.
     * @return const StringLTypeDefn instance.
     */
    FASTDDS_EXPORTED_API static const StringLTypeDefn build_string_l_type_defn(
            LBound bound);

    /**
     * @brief Build PlainCollectionHeader instance.
     *
     * @param [in] equiv_kind EquivalenceKind: EK_MINIMAL/EK_COMPLETE/EK_BOTH
     * @param [in] element_flags CollectionElementFlags to be set. This element must be constructed with the corresponding
     *                      builder to ensure its consistency.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError if the given element_flags are inconsistent.
     *            This exception is only thrown in Debug build mode.
     * @return const PlainCollectionHeader instance.
     */
    FASTDDS_EXPORTED_API static const PlainCollectionHeader build_plain_collection_header(
            EquivalenceKind equiv_kind,
            CollectionElementFlag element_flags);

    /**
     * @brief Build PlainSequenceSElemDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] s_bound Sequence bound.
     * @param [in] element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. The given bound is 0.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
     * @return const PlainSequenceSElemDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainSequenceSElemDefn build_plain_sequence_s_elem_defn(
            const PlainCollectionHeader& header,
            SBound s_bound,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier);

    /**
     * @brief Build PlainSequenceLElemDefn instance.
     *
     * @pre bound > 255
     * @pre element_identifier has been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] l_bound Sequence bound.
     * @param [in] element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. Bound lower than 256.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
     * @return const PlainSequenceLElemDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainSequenceLElemDefn build_plain_sequence_l_elem_defn(
            const PlainCollectionHeader& header,
            LBound l_bound,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier);

    /**
     * @brief Add dimension bound to the array bound sequence.
     *
     * @tparam array Either a SBoundSeq or LBoundSeq.
     * @tparam element Either a SBound or LBound.
     * @param [in,out] array_bound_seq Sequence with the array bounds.
     * @param [in] dimension_bound Dimension bound to be added into the sequence.
     */
    template<typename element>
    static void add_array_dimension(
            std::vector<element>& array_bound_seq,
            element dimension_bound)
    {
        if (dimension_bound == INVALID_LBOUND)
        {
            throw InvalidArgumentError("bound parameter must be greater than 0");
        }
        array_bound_seq.push_back(dimension_bound);
    }

    /**
     * @brief Build PlainArraySElemDefn instance.
     *
     * @pre Any element in array_bound_seq must be greater than 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] array_bound_seq Bounds for the array dimensions.
     * @param [in] element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
     * @return const PlainArraySElemDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainArraySElemDefn build_plain_array_s_elem_defn(
            const PlainCollectionHeader& header,
            const SBoundSeq& array_bound_seq,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier);

    /**
     * @brief Build PlainArrayLElemDefn instance.
     *
     * @pre At least one element of array_bound_seq must be greater than 255 and no element must be 0 (INVALID_SBOUND)
     * @pre element_identifier has been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] array_bound_seq Bounds for the array dimensions.
     * @param [in] element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. There is no dimension with a bound greater than 255.
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent header (only in Debug build mode).
     *              5. Inconsistent element_identifier (only in Debug build mode).
     * @return const PlainArrayLElemDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainArrayLElemDefn build_plain_array_l_elem_defn(
            const PlainCollectionHeader& header,
            const LBoundSeq& array_bound_seq,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier);

    /**
     * @brief Build PlainMapSTypeDefn instance.
     *
     * @pre bound > 0 (INVALID_SBOUND)
     * @pre Both element_identifier and key_identifier have been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] bound Map bound.
     * @param [in] element_identifier Map element TypeIdentifier.
     * @param [in] key_flags Flags applying to map key.
     * @param [in] key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. Given bound is zero (INVALID_SBOUND)
     *              2. Inconsistent element_identifier EquivalenceKind with the one contained in the header.
     *              3. Direct hash key_identifier or indirect hash TypeIdentifier with exception to string/wstring.
     *                 XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements
     *                 of signed and unsigned integer types and of narrow and wide string types.
     *              4. Inconsistent key_flags.
     *              5. Inconsistent header (only in Debug build mode).
     *              6. Inconsistent element_identifier or key_identifier (only in Debug build mode).
     * @return const PlainMapSTypeDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainMapSTypeDefn build_plain_map_s_type_defn(
            const PlainCollectionHeader& header,
            const SBound bound,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier,
            const CollectionElementFlag key_flags,
            const eprosima::fastcdr::external<TypeIdentifier>& key_identifier);

    /**
     * @brief Build PlainMapLTypeDefn instance.
     *
     * @pre bound > 255
     * @pre Both element_identifier and key_identifier have been initialized.
     * @param [in] header PlainCollectionHeader to be set.
     * @param [in] bound Map bound.
     * @param [in] element_identifier Map element TypeIdentifier.
     * @param [in] key_flags Flags applying to map key.
     * @param [in] key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception
     *              1. Given bound is lower than 256
     *              2. Inconsistent element_identifier EquivalenceKind with the one contained in the header.
     *              3. Direct hash key_identifier or indirect hash TypeIdentifier with exception to string/wstring.
     *                 XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements
     *                 of signed and unsigned integer types and of narrow and wide string types.
     *              4. Inconsistent key_flags.
     *              5. Inconsistent header (only in Debug build mode).
     *              6. Inconsistent element_identifier or key_identifier (only in Debug build mode).
     * @return const PlainMapLTypeDefn instance.
     */
    FASTDDS_EXPORTED_API static const PlainMapLTypeDefn build_plain_map_l_type_defn(
            const PlainCollectionHeader& header,
            const LBound bound,
            const eprosima::fastcdr::external<TypeIdentifier>& element_identifier,
            const CollectionElementFlag key_flags,
            const eprosima::fastcdr::external<TypeIdentifier>& key_identifier);

    /**
     * @brief Build StronglyConnectedComponentId instance.
     *
     * @param [in] sc_component_id Strongly Connected Component (SCC) ID.
     * @param [in] scc_length Number of components within SCC.
     * @param [in] scc_index Identify specific component within SCC.
     * @return const StronglyConnectedComponentId instance.
     */
    FASTDDS_EXPORTED_API static const StronglyConnectedComponentId build_strongly_connected_component_id(
            const TypeObjectHashId& sc_component_id,
            int32_t scc_length,
            int32_t scc_index);

    /**
     * @brief Build ExtendedTypeDefn instance (empty. Available for future extension).
     *
     * @return const ExtendedTypeDefn instance.
     */
    FASTDDS_EXPORTED_API static const ExtendedTypeDefn build_extended_type_defn();

    //}}}

    //{{{ Register Indirect Hash TypeIdentifiers

    /**
     * Primitive types are registered when TypeObjectRegistry is instantiated.
     */

    /**
     * @brief Register small string/wstring TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] string StringSTypeDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the StringSTypeDefn just registered.
     * @param [in] wstring Flag to build a wstring. Default false.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_s_string_type_identifier(
            const StringSTypeDefn& string,
            const std::string& type_name,
            TypeIdentifierPair& type_ids,
            bool wstring = false);

    /**
     * @brief Register large string/wstring TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] string StringLTypeDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the StringLTypeDefn just registered.
     * @param [in] wstring Flag to build a wstring. Default false.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_l_string_type_identifier(
            const StringLTypeDefn& string,
            const std::string& type_name,
            TypeIdentifierPair& type_ids,
            bool wstring = false);

    /**
     * @brief Register small sequence TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_seq PlainSequenceSElemDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainSequenceSElemDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_s_sequence_type_identifier(
            const PlainSequenceSElemDefn& plain_seq,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register large sequence TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_seq PlainSequenceLElemDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainSequenceLElemDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_l_sequence_type_identifier(
            const PlainSequenceLElemDefn& plain_seq,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register small array TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_array PlainArraySElemDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainArraySElemDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_s_array_type_identifier(
            const PlainArraySElemDefn& plain_array,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register large array TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_array PlainArrayLElemDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainArrayLElemDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_l_array_type_identifier(
            const PlainArrayLElemDefn& plain_array,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register small map TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_map PlainMapSTypeDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainMapSTypeDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_s_map_type_identifier(
            const PlainMapSTypeDefn& plain_map,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register large map TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] plain_map PlainMapLTypeDefn union member to set.
     * @param [in] type_name Type name to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the PlainMapLTypeDefn just registered.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_l_map_type_identifier(
            const PlainMapLTypeDefn& plain_map,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register StronglyConnectedComponent TypeIdentifier into TypeObjectRegistry.
     *
     * @param [in] scc StronglyConnectedComponent union member to set.
     * @param [in] type_name Type name to be registered.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_scc_type_identifier(
            const StronglyConnectedComponentId& scc,
            const std::string& type_name);
    //}}}

    //{{{ Annotation usage
    /**
     * @brief Build ExtendedAnnotationParameterValue instance (empty. Available for future extension).
     *
     * @return const ExtendedAnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const ExtendedAnnotationParameterValue build_extended_annotation_parameter_value();

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] boolean_value Boolean value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            bool boolean_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] byte_value Byte value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value_byte(
            uint8_t byte_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] int8_value Int8 value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            int8_t int8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] uint8_value Unsigned int8 value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            uint8_t uint8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] int16_value Short value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            int16_t int16_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] uint16_value Unsigned short value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            uint16_t uint16_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] int32_value Long value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            int32_t int32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] uint32_value Unsigned long value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            uint32_t uint32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] int64_value Long long value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            int64_t int64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] uint64_value Unsigned long long value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            uint64_t uint64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] float32_value Float value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            float float32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] float64_value Double value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            double float64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] float128_value Long double value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            long double float128_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] char_value Char value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            char char_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] wchar_value Wide char value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            wchar_t wchar_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] enumerated_value Enumerated value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value_enum(
            int32_t enumerated_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] string8_value String value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            const eprosima::fastcdr::fixed_string<128>& string8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param [in] string16_value Wide string value to set in the union.
     * @return const AnnotationParameterValue instance.
     */
    FASTDDS_EXPORTED_API static const AnnotationParameterValue build_annotation_parameter_value(
            const std::wstring& string16_value);

    /**
     * @brief Build AppliedAnnotationParameter instance.
     *
     * @param [in] paramname_hash Parameter name hash.
     * @param [in] value Parameter value.
     * @return const AppliedAnnotationParameter instance.
     */
    FASTDDS_EXPORTED_API static const AppliedAnnotationParameter build_applied_annotation_parameter(
            const NameHash& paramname_hash,
            const AnnotationParameterValue& value);

    /**
     * @brief Add AppliedAnnotationParameter to the sequence.
     *
     * @param [in,out] param_seq AppliedAnnotationParameter sequence to be modified.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the parameter being added has
     *            already been included in the sequence.
     * @param [in] param AppliedAnnotationParameter to be added.
     */
    FASTDDS_EXPORTED_API static void add_applied_annotation_parameter(
            AppliedAnnotationParameterSeq& param_seq,
            const AppliedAnnotationParameter& param);

    /**
     * @brief Build AppliedAnnotation instance.
     *
     * @param [in] annotation_typeid Annotation TypeIdentifier.
     * @param [in] param_seq Annotation parameters.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given annotation_typeid TypeIdentifier does not correspond to an annotation TypeObject (only in
     *                 Debug build mode).
     *              2. Given AppliedAnnotationParameterSeq is inconsistent (only in Debug build mode).
     *              3. Given annotation TypeIdentifier corresponds to a builtin annotation and the given parameters are
     *                 inconsistent (only in Debug build mode).
     * @return const AppliedAnnotation instance.
     */
    FASTDDS_EXPORTED_API static const AppliedAnnotation build_applied_annotation(
            const TypeIdentifier& annotation_typeid,
            const eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>& param_seq);

    /**
     * @brief Add AppliedAnnotation to the sequence.
     *
     * @param [in,out] ann_custom_seq AppliedAnnotation sequence to be modified.
     * @param [in] ann_custom AppliedAnnotation to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given AppliedAnnotation is not consistent (only in Debug build mode).
     *              2. Given AppliedAnnotation is already present in the sequence.
     */
    FASTDDS_EXPORTED_API static void add_applied_annotation(
            AppliedAnnotationSeq& ann_custom_seq,
            const AppliedAnnotation& ann_custom);

    /**
     * @brief Build AppliedVerbatimAnnotation instance.
     *
     * @param [in] placement Verbatim annotation placement parameter.
     * @param [in] language Verbatim annotation language parameter.
     * @param [in] text Verbatim annotation text parameter.
     * @return const AppliedVerbatimAnnotation instance.
     */
    FASTDDS_EXPORTED_API static const AppliedVerbatimAnnotation build_applied_verbatim_annotation(
            PlacementKind placement,
            const eprosima::fastcdr::fixed_string<32>& language,
            const std::string& text);
    //}}}

    //{{{ Aggregate types
    /**
     * @brief Build AppliedBuiltinMemberAnnotations instance.
     *
     * @param [in] unit Unit annotation value.
     * @param [in] min Min annotation value.
     * @param [in] max Max annotation value.
     * @param [in] hash_id Hashid annotation value.
     * @return const AppliedBuiltinMemberAnnotations instance.
     */
    FASTDDS_EXPORTED_API static const AppliedBuiltinMemberAnnotations build_applied_builtin_member_annotations(
            const eprosima::fastcdr::optional<std::string>& unit,
            const eprosima::fastcdr::optional<AnnotationParameterValue>& min,
            const eprosima::fastcdr::optional<AnnotationParameterValue>& max,
            const eprosima::fastcdr::optional<std::string>& hash_id);

    /**
     * @brief Build CommonStructMember instance.
     *
     * @param [in] member_id Member identifier.
     * @param [in] member_flags Member flags: optional, must_understand, key, and external.
     * @param [in] member_type_id Member TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception:
     *              1. The given flags are not consistent (only in Debug build mode).
     *              2. The given TypeIdentifier is not consistent (only in Debug build mode).
     * @return const CommonStructMember instance.
     */
    FASTDDS_EXPORTED_API static const CommonStructMember build_common_struct_member(
            MemberId member_id,
            StructMemberFlag member_flags,
            const TypeIdentifier& member_type_id);

    /**
     * @brief Build CompleteMemberDetail instance.
     *
     * @param [in] name Member name.
     * @param [in] ann_builtin Member builtin annotations.
     * @param [in] ann_custom Member custom annotations.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Empty member name.
     *              2. Given AppliedAnnotationSeq is not consistent (only Debug build mode).
     * @return const CompleteMemberDetail instance.
     */
    FASTDDS_EXPORTED_API static const CompleteMemberDetail build_complete_member_detail(
            const MemberName& name,
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * MinimalMemberDetail constructed from CompleteMemberDetail
     */

    /**
     * @brief Build CompleteStructMember instance.
     *
     * @param [in] common CommonStructMember to be set.
     * @param [in] detail CompleteMemberDetail to be set.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonStructMember is inconsistent (only Debug build mode).
     *              2. Given CompleteMemberDetail is inconsistent (only Debug build mode).
     * @return const CompleteMemberDetail instance.
     */
    FASTDDS_EXPORTED_API static const CompleteStructMember build_complete_struct_member(
            const CommonStructMember& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add CompleteStructMember to the sequence.
     *
     * @param [in,out] member_seq CompleteStructMember sequence to be modified.
     * @param [in] member CompleteStructMember to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CompleteStructMember is not consistent (only in Debug build mode).
     *              2. There is already another member in the sequence with the same member id or the same member name
     *                 (only in Debug build mode).
     */
    FASTDDS_EXPORTED_API static void add_complete_struct_member(
            CompleteStructMemberSeq& member_seq,
            const CompleteStructMember& member);

    /**
     * MinimalStructMember constructed from CompleteStructMember
     */

    /**
     * @brief Build AppliedBuiltinTypeAnnotations instance.
     *
     * @param [in] verbatim AppliedVerbatimAnnotation to be set.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given verbatim annotation
     *            is inconsistent (only in Debug build mode).
     * @return const AppliedBuiltinTypeAnnotations instance.
     */
    FASTDDS_EXPORTED_API static const AppliedBuiltinTypeAnnotations build_applied_builtin_type_annotations(
            const eprosima::fastcdr::optional<AppliedVerbatimAnnotation>& verbatim);

    /**
     * MinimalTypeDetail constructed from CompleteTypeDetail: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteTypeDetail instance.
     *
     * @param [in] ann_builtin Verbatim annotation.
     * @param [in] ann_custom Applied annotations.
     * @param [in] type_name Name of the type.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. any applied annotation is not consistent (only Debug build mode).
     * @return const CompleteTypeDetail instance.
     */
    FASTDDS_EXPORTED_API static const CompleteTypeDetail build_complete_type_detail(
            const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom,
            const QualifiedTypeName& type_name);

    /**
     * @brief Build CompleteStructHeader instance.
     *
     * @param [in] base_type TypeIdentifier of the parent structure (inheritance).
     * @param [in] detail CompleteTypeDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given TypeIdentifier is not consistent (direct HASH or empty TypeIdentifier). In Debug build
     *                 mode the corresponding TypeObject is also checked in case of direct HASH TypeIdentifier.
     *              2. Given CompleteTypeDetail is not consistent (only in Debug build mode).
     * @return const CompleteStructHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteStructHeader build_complete_struct_header(
            const TypeIdentifier& base_type,
            const CompleteTypeDetail& detail);

    /**
     * MinimalStructHeader constructed from CompleteStructHeader.
     */

    /**
     * @brief Build CompleteStructType instance.
     *
     * @param [in] struct_flags StructTypeFlags.
     * @param [in] header CompleteStructHeader.
     * @param [in] member_seq Sequence of CompleteStructMembers.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given StructTypeFlag is not consistent (only in Debug build mode).
     *              2. Given CompleteStructHeader is not consistent (only in Debug build mode).
     *              3. Given CompleteStructMemberSeq is not consistent (only in Debug build mode).
     *              4. Given flags are not consistent with the builtin annotations.
     * @return const CompleteStructType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteStructType build_complete_struct_type(
            StructTypeFlag struct_flags,
            const CompleteStructHeader& header,
            const CompleteStructMemberSeq& member_seq);

    /**
     * MinimalStructType constructed from CompleteStructType.
     */
    //}}}

    //{{{ Union

    /**
     * @brief Add label to the union case label sequence.
     *
     * @param [in,out] label_seq Sequence to be modified.
     * @param [in] label Label to be added.
     */
    FASTDDS_EXPORTED_API static void add_union_case_label(
            UnionCaseLabelSeq& label_seq,
            int32_t label);

    /**
     * @brief Build CommonUnionMember instance.
     *
     * @param [in] member_id Member identifier.
     * @param [in] member_flags Member flags.
     * @param [in] type_id Member TypeIdentifier.
     * @param [in] label_seq Member applicable case labels.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given UnionMemberFlags are not consistent (only Debug build mode).
     *              2. Given TypeIdentifier is not consistent (only Debug build mode).
     * @return const CommonUnionMember instance.
     */
    FASTDDS_EXPORTED_API static const CommonUnionMember build_common_union_member(
            MemberId member_id,
            UnionMemberFlag member_flags,
            const TypeIdentifier& type_id,
            const UnionCaseLabelSeq& label_seq);

    /**
     * @brief Build CompleteUnionMember instance.
     *
     * @param [in] common CommonUnionMember.
     * @param [in] detail CompleteMemberDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonUnionMember is not consistent (only in Debug build mode).
     *              2. Given CompleteMemberDetail is not consistent (only in Debug build mode).
     * @return const CompleteUnionMember instance.
     */
    FASTDDS_EXPORTED_API static const CompleteUnionMember build_complete_union_member(
            const CommonUnionMember& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add CompleteUnionMember to sequence.
     *
     * @param [in,out] complete_union_member_seq Sequence to be modified.
     * @param [in] member Complete union member to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CompleteUnionMember is not consistent (only in Debug build mode).
     *              2. There is already another member in the sequence with the same member id or the same member name
     *                 (only in Debug build mode).
     *              3. If the given member is marked as default, if there is already another member marked as default
     *                 (only in Debug build mode).
     *              4. There are repeated union case labels (only in Debug build mode).
     *              5. Member name is protected ("discriminator").
     */
    FASTDDS_EXPORTED_API static void add_complete_union_member(
            CompleteUnionMemberSeq& complete_union_member_seq,
            const CompleteUnionMember& member);

    /**
     * MinimalUnionMember constructed from CompleteUnionMember.
     */

    /**
     * @brief Build CommonDiscriminatorMember instance.
     *
     * @param [in] member_flags Discriminator flags.
     * @param [in] type_id Discriminator TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given discriminator flags are inconsistent (only in Debug build mode).
     *              2. Given TypeIdentifier is not consistent.
     *                 XTypes v1.3 Clause 7.2.2.4.4.3 The discriminator of a union must be one of the following types:
     *                 Boolean, Byte, Char8, Char16, Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64, Uint64, any
     *                 enumerated type, any alias type that resolves, directly or indirectly, to one of the
     *                 aforementioned types.
     * @return const CommonDiscriminatorMember instance.
     */
    FASTDDS_EXPORTED_API static const CommonDiscriminatorMember build_common_discriminator_member(
            UnionDiscriminatorFlag member_flags,
            const TypeIdentifier& type_id);

    /**
     * @brief Build CompleteDiscriminatorMember instance.
     *
     * @param [in] common CommonDiscriminatorMember.
     * @param [in] ann_builtin Verbatim annotation.
     * @param [in] ann_custom Applied annotations.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonDiscriminatorMember is inconsistent (only in Debug build mode).
     *              2. AppliedBuiltinTypeAnnotation is inconsistent (only in Debug build mode).
     *              3. Any given AppliedAnnotation is inconsistent (only in Debug build mode).
     *              4. CommonDiscriminatorMember is inconsistent with given builtin annotations.
     * @return const CompleteDiscriminatorMember instance.
     */
    FASTDDS_EXPORTED_API static const CompleteDiscriminatorMember build_complete_discriminator_member(
            const CommonDiscriminatorMember& common,
            const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * MinimalDiscriminatorMember constructed from CompleteDiscriminatorMember.
     */

    /**
     * @brief Build CompleteUnionHeader instance.
     *
     * @param detail CompleteTypeDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            not consistent (only in Debug build mode).
     * @return const CompleteUnionHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteUnionHeader build_complete_union_header(
            const CompleteTypeDetail& detail);

    /**
     * MinimalUnionHeader constructed from CompleteUnionHeader.
     */

    /**
     * @brief Build CompleteUnionType instance.
     *
     * @param [in] union_flags
     * @param [in] header
     * @param [in] discriminator
     * @param [in] member_seq
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given UnionTypeFlags are not consistent (only in Debug build mode).
     *              2. Given CompleteUnionHeader is not consistent (only in Debug build mode).
     *              3. Given CompleteDiscriminatorMember inconsistent (only in Debug build mode).
     *              4. Given CompleteUnionMemberSeq is not consistent (only in Debug build mode).
     *              5. Given flags are not consistent with the builtin annotations.
     * @return const CompleteUnionType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteUnionType build_complete_union_type(
            UnionTypeFlag union_flags,
            const CompleteUnionHeader& header,
            const CompleteDiscriminatorMember& discriminator,
            const CompleteUnionMemberSeq& member_seq);

    /**
     * MinimalUnionType constructed from CompleteUnionType.
     */
    //}}}

    //{{{ Annotation

    /**
     * @brief Build CommonAnnotationParameter instance.
     *
     * @param [in] member_flags AnnotationParameterFlag: empty. No flags apply. It must be zero.
     * @param [in] member_type_id Member TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given AnnotationParameterFlag are not empty.
     *              2. Given TypeIdentifier is not consistent (only in Debug build mode).
     * @return const CommonAnnotationParameter instance.
     */
    FASTDDS_EXPORTED_API static const CommonAnnotationParameter build_common_annotation_parameter(
            AnnotationParameterFlag member_flags,
            const TypeIdentifier& member_type_id);

    /**
     * @brief Build CompleteAnnotationParameter instance.
     *
     * @param [in] common CommonAnnotationParameter.
     * @param [in] name Member name.
     * @param [in] default_value Annotation default value.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonAnnotationParameter is inconsistent (only in Debug build mode).
     *              2. CommonAnnotationParameter TypeIdentifier is inconsistent with AnnotationParameterValue type.
     *              3. Given parameter name is empty.
     * @return const CompleteAnnotationParameter instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAnnotationParameter build_complete_annotation_parameter(
            const CommonAnnotationParameter& common,
            const MemberName& name,
            const AnnotationParameterValue& default_value);

    /**
     * @brief Add CompleteAnnotationParameter to sequence.
     *
     * @param [in,out] sequence Sequence to be modified.
     * @param [in] param Complete annotation parameter to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CompleteAnnotationParameter is not consistent (only in Debug build mode).
     *              2. There is already another member in the sequence with the same member id or the same member name
     *                 (only in Debug build mode).
     */
    FASTDDS_EXPORTED_API static void add_complete_annotation_parameter(
            CompleteAnnotationParameterSeq& sequence,
            const CompleteAnnotationParameter& param);

    /**
     * MinimalAnnotationParameter constructed from CompleteAnnotationParameter.
     */

    /**
     * @brief Build CompleteAnnotationHeader instance.
     *
     * @param [in] annotation_name Qualified annotation type name.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError if the annotation_name is empty.
     * @return const CompleteAnnotationHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAnnotationHeader build_complete_annotation_header(
            const QualifiedTypeName& annotation_name);

    /**
     * MinimalAnnotationHeader constructed from CompleteAnnotationHeader: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteAnnotationType instance.
     *
     * @param [in] annotation_flag Unused. No flags apply. It must be 0.
     * @param [in] header CompleteAnnotationHeader.
     * @param [in] member_seq CompleteAnnotationParameter sequence.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any annotation flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Any CompleteAnnotationParameter in the sequence is inconsistent (only in Debug build mode).
     * @return const CompleteAnnotationType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAnnotationType build_complete_annotation_type(
            AnnotationTypeFlag annotation_flag,
            const CompleteAnnotationHeader& header,
            const CompleteAnnotationParameterSeq& member_seq);

    /**
     * MinimalAnnotationType constructed from CompleteAnnotationType.
     */
    //}}}

    //{{{ Alias

    /**
     * @brief Build CommonAliasBody instance.
     *
     * @param [in] related_flags AliasMemberFlag: unused. No flags apply. It must be 0.
     * @param [in] related_type Related TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any alias member flag is set.
     *              2. Non-consistent TypeIdentifier (only in Debug build mode).
     * @return const CommonAliasBody instance.
     */
    FASTDDS_EXPORTED_API static const CommonAliasBody build_common_alias_body(
            AliasMemberFlag related_flags,
            const TypeIdentifier& related_type);

    /**
     * @brief Build CompleteAliasBody instance.
     *
     * @param [in] common CommonAliasBody.
     * @param [in] ann_builtin Applied builtin member annotations: unit, max, min, range, hashid
     * @param [in] ann_custom Applied custom annotations
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonAliasBody is inconsistent (only Debug build mode).
     *              2. AppliedAnnotationSeq is inconsistent (only Debug build mode).
     *              3. hashid builtin annotation is set.
     * @return const CompleteAliasBody instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAliasBody build_complete_alias_body(
            const CommonAliasBody& common,
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * MinimalAliasBody constructed from CompleteAliasBody.
     */

    /**
     * @brief Build CompleteAliasHeader instance.
     *
     * @param [in] detail Complete type detail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            inconsistent (only in Debug build mode).
     * @return const CompleteAliasHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAliasHeader build_complete_alias_header(
            const CompleteTypeDetail& detail);

    /**
     * MinimalAliasHeader constructed from CompleteAliasHeader: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteAliasType instance.
     *
     * @param [in] alias_flags Alias type flags: unused. No flags apply. It must be zero.
     * @param [in] header CompleteAliasHeader.
     * @param [in] body CompleteAliasBody.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any alias type flag is set.
     *              2. Inconsistent header and/or body (only in Debug build mode).
     * @return const CompleteAliasType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteAliasType build_complete_alias_type(
            AliasTypeFlag alias_flags,
            const CompleteAliasHeader& header,
            const CompleteAliasBody& body);

    /**
     * MinimalAliasType constructed from CompleteAliasType.
     */
    //}}}

    //{{{ Collections

    /**
     * @brief Build CompleteElementDetail instance.
     *
     * @param [in] ann_builtin Applied builtin member annotations: unit, max, min, range, hashid
     * @param [in] ann_custom Applied custom annotations
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. AppliedAnnotationSeq is inconsistent (only Debug build mode).
     *              2. hashid builtin annotation is applied.
     * @return const CompleteElementDetail instance.
     */
    FASTDDS_EXPORTED_API static const CompleteElementDetail build_complete_element_detail(
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * @brief Build CommonCollectionElement instance.
     *
     * @param [in] element_flags CollectionElementFlag.
     * @param [in] type TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given collection element flags are not consistent (only in Debug build mode).
     *              2. Given TypeIdentifier is not consistent (only in Debug build mode).
     * @return const CommonCollectionElement instance
     */
    FASTDDS_EXPORTED_API static const CommonCollectionElement build_common_collection_element(
            CollectionElementFlag element_flags,
            const TypeIdentifier& type);

    /**
     * @brief Build CompleteCollectionElement instance.
     *
     * @param [in] common CommonCollectionElement.
     * @param [in] detail CompleteElementDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonCollectionElement is not consistent (only in Debug build mode).
     *              2. Given CompleteElementDetail is not consistent (only in Debug build mode).
     * @return const CompleteCollectionElement instance
     */
    FASTDDS_EXPORTED_API static const CompleteCollectionElement build_complete_collection_element(
            const CommonCollectionElement& common,
            const CompleteElementDetail& detail);

    /**
     * MinimalCollectionElement constructed from CompleteCollectionElement.
     */

    /**
     * @brief Build CommonCollectionHeader instance.
     *
     * @param [in] bound Collection bound.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given bound is not
     *            consistent.
     * @return const CommonCollectionHeader instance.
     */
    FASTDDS_EXPORTED_API static const CommonCollectionHeader build_common_collection_header(
            LBound bound);

    /**
     * @brief Build CompleteCollectionHeader instance.
     *
     * @param [in] common CommonCollectionHeader
     * @param [in] detail CompleteTypeDetail
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonCollectionHeader is inconsistent (only in Debug build mode).
     *              2. Given CompleteTypeDetail is inconsistent (only in Debug build mode).
     * @return const CompleteCollectionHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteCollectionHeader build_complete_collection_header(
            const CommonCollectionHeader& common,
            const eprosima::fastcdr::optional<CompleteTypeDetail>& detail);

    /**
     * MinimalCollectionHeader constructed from CompleteCollectionHeader.
     */
    //}}}

    //{{{ Sequence

    /**
     * @brief Build CompleteSequenceType instance.
     *
     * @param [in] collection_flag collection type flag: unused. No flags apply. It must be 0.
     * @param [in] header CompleteCollectionHeader.
     * @param [in] element CompleteCollectionElement.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any collection flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Given element is inconsistent (only in Debug build mode).
     * @return const CompleteSequenceType instance
     */
    FASTDDS_EXPORTED_API static const CompleteSequenceType build_complete_sequence_type(
            CollectionTypeFlag collection_flag,
            const CompleteCollectionHeader& header,
            const CompleteCollectionElement& element);

    /**
     * MinimalSequenceType constructed from CompleteSequenceType.
     */
    //}}}

    //{{{ Array

    /**
     * @brief Build CommonArrayHeader instance.
     *
     * @param [in] bound_seq Sequence of the dimension's bounds.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if any given bound is 0 (invalid).
     * @return const CommonArrayHeader instance.
     */
    FASTDDS_EXPORTED_API static const CommonArrayHeader build_common_array_header(
            const LBoundSeq& bound_seq);

    /**
     * @brief Build CompleteArrayHeader instance.
     *
     * @param [in] common CommonArrayHeader.
     * @param [in] detail CompleteTypeDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonArrayHeader is inconsistent (only in Debug build mode).
     *              2. Given CompleteTypeDetail is inconsistent (only in Debug build mode).
     * @return const CompleteArrayHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteArrayHeader build_complete_array_header(
            const CommonArrayHeader& common,
            const CompleteTypeDetail& detail);

    /**
     * MinimalArrayHeader constructed from CompleteArrayHeader.
     */

    /**
     * @brief Build CompleteArrayType instance.
     *
     * @param [in] collection_flag collection type flag: unused. No flags apply. It must be 0.
     * @param [in] header CompleteArrayHeader.
     * @param [in] element CompleteCollectionElement.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any collection flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Given element is inconsistent (only in Debug build mode).
     * @return const CompleteArrayType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteArrayType build_complete_array_type(
            CollectionTypeFlag collection_flag,
            const CompleteArrayHeader& header,
            const CompleteCollectionElement& element);

    /**
     * MinimalArrayType constructed from CompleteArrayType.
     */
    //}}}

    //{{{ Map

    /**
     * @brief Build CompleteMapType instance.
     *
     * @param [in] collection_flag collection type flag: unused. No flags apply. It must be 0.
     * @param [in] header CompleteArrayHeader.
     * @param [in] key CompleteCollectionElement describing map key.
     * @param [in] element CompleteCollectionElement describing map element.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any collection flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Given key TypeIdentifier is inconsistent.
     *              4. Given key description is inconsistent (only in Debug build mode).
     *              5. Given element is inconsistent (only in Debug build mode).
     * @return const CompleteMapType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteMapType build_complete_map_type(
            CollectionTypeFlag collection_flag,
            const CompleteCollectionHeader& header,
            const CompleteCollectionElement& key,
            const CompleteCollectionElement& element);

    /**
     * MinimalMapType constructed from CompleteMapType.
     */
    //}}}

    //{{{ Enumeration

    /**
     * @brief Build CommonEnumeratedLiteral instance.
     *
     * @param [in] value Enumerated literal value.
     * @param [in] flags Enumerated literal flags: only default flag apply.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if any other flag different from
     *            default is set (only in Debug build mode).
     * @return const CommonEnumeratedLiteral instance.
     */
    FASTDDS_EXPORTED_API static const CommonEnumeratedLiteral build_common_enumerated_literal(
            int32_t value,
            EnumeratedLiteralFlag flags);

    /**
     * @brief Build CompleteEnumeratedLiteral instance.
     *
     * @param [in] common CommonEnumeratedLiteral.
     * @param [in] detail CompleteMemberDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonEnumeratedLiteral is inconsistent (only in Debug build mode).
     *              2. Given CompleteMemberDetail is inconsistent (only in Debug build mode).
     * @return const CompleteEnumeratedLiteral instance.
     */
    FASTDDS_EXPORTED_API static const CompleteEnumeratedLiteral build_complete_enumerated_literal(
            const CommonEnumeratedLiteral& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add CompleteEnumeratedLiteral to sequence.
     *
     * @param [in] sequence Sequence to be modified.
     * @param [in,out] enum_literal CompleteEnumeratedLiteral to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonEnumeratedLiteral is not consistent (only in Debug build mode).
     *              2. There is already another literal in the sequence with the same value or the same member name
     *                 (only in Debug build mode).
     */
    FASTDDS_EXPORTED_API static void add_complete_enumerated_literal(
            CompleteEnumeratedLiteralSeq& sequence,
            const CompleteEnumeratedLiteral& enum_literal);

    /**
     * MinimalEnumeratedLiteral constructed from CompleteEnumeratedLiteral.
     */

    /**
     * @brief Build CommonEnumeratedHeader instance.
     *
     * @param [in] bit_bound XTypes v1.3 Clause 7.3.1.2.1.5 It is important to note that the value member of the
     *                      [bit_bound] annotation may take any value from 1 to 32, inclusive, when this annotation is
     *                      applied to an enumerated type.
     * @param [in] bitmask Flag in case that the header being built corresponds to a Bitmask. By default is false.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given bit_bound is not
     *            consistent.
     * @return const CommonEnumeratedHeader instance.
     */
    FASTDDS_EXPORTED_API static const CommonEnumeratedHeader build_common_enumerated_header(
            BitBound bit_bound,
            bool bitmask = false);

    /**
     * @brief Build CompleteEnumeratedHeader instance.
     *
     * @param [in] common CommonEnumeratedHeader.
     * @param [in] detail CompleteTypeDetail.
     * @param [in] bitmask flag set if the given header corresponds to a bitmask. Only required in Debug build mode.
     *                    Set to false by default.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonEnumeratedHeader is inconsistent (only in Debug build mode).
     *              2. Given CompleteTypeDetail is inconsistent (only in Debug build mode).
     * @return const CompleteEnumeratedHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteEnumeratedHeader build_complete_enumerated_header(
            const CommonEnumeratedHeader& common,
            const CompleteTypeDetail& detail,
            bool bitmask = false);

    /**
     * MinimalEnumeratedHeader constructed from CompleteEnumeratedHeader.
     */

    /**
     * @brief Build CompleteEnumeratedType instance.
     *
     * @param [in] enum_flags Enumeration flags: unused. No flags apply. It must be 0.
     * @param [in] header CompleteEnumeratedHeader.
     * @param [in] literal_seq Sequence of CompleteEnumeratedLiterals.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any flag is set.
     *              2. Given CompleteEnumeratedHeader is inconsistent (only in Debug build mode).
     *              3. Given CompleteEnumeratedLiteralSeq is inconsistent (only in Debug build mode).
     * @return const CompleteEnumeratedType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteEnumeratedType build_complete_enumerated_type(
            EnumTypeFlag enum_flags,
            const CompleteEnumeratedHeader& header,
            const CompleteEnumeratedLiteralSeq& literal_seq);

    /**
     * MinimalEnumeratedType constructed from CompleteEnumeratedType.
     */
    //}}}

    //{{{ Bitmask

    /**
     * @brief Build CommonBitflag instance.
     *
     * @param [in] position Bit position in the bitmask.
     * @param [in] flags Bit flags: unused. No flags apply. It must be 0.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. any given flag is set.
     *              2. given position is inconsistent. XTypes v1.3 Clause 7.2.2.4.1.2 Each bit in this subset is
     *                 identified by name and by an index, numbered from 0 to (bound-1). The bound must be greater than
     *                 zero and no greater than 64.
     * @return const CommonBitflag instance.
     */
    FASTDDS_EXPORTED_API static const CommonBitflag build_common_bitflag(
            uint16_t position,
            BitflagFlag flags);

    /**
     * @brief Build CompleteBitflag instance.
     *
     * @param [in] common CommonBitflag.
     * @param [in] detail CompleteMemberDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonBitflag is inconsistent (only in Debug build mode).
     *              2. Given CompleteMemberDetail is inconsistent (only in Debug build mode).
     *              3. Non-applicable builtin annotations applied.
     * @return const CompleteBitflag instance.
     */
    FASTDDS_EXPORTED_API static const CompleteBitflag build_complete_bitflag(
            const CommonBitflag& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add complete bitflag to the sequence.
     *
     * @param [in,out] sequence Sequence to be modified.
     * @param [in] bitflag CompleteBitflag to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given bitflag is inconsistent (only in Debug build mode).
     *              2. There is already another bitflag in the sequence with the same position or the same name
     *                 (only in Debug build mode).
     */
    FASTDDS_EXPORTED_API static void add_complete_bitflag(
            CompleteBitflagSeq& sequence,
            const CompleteBitflag& bitflag);

    /**
     * MinimalBitflag constructed from CompleteBitflag.
     */

    /**
     * CommonBitmaskHeader is not used.
     * CompleteBitmaskHeader is defined as CompleteEnumeratedHeader.
     * MinimalBitmaskHeader is defined as MinimalEnumeratedHeader.
     */

    /**
     * @brief Build CompleteBitmaskType instance.
     *
     * @param [in] bitmask_flags Bitmask flags: unused. No flags apply. It must be 0.
     * @param [in] header CompleteBitmaskHeader/CompleteEnumeratedHeader
     * @param [in] flag_seq Sequence of CompleteBitflag.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. any given flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Given Bitflag sequence is inconsistent (only in Debug build mode).
     * @return const CompleteBitmaskType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteBitmaskType build_complete_bitmask_type(
            BitmaskTypeFlag bitmask_flags,
            const CompleteBitmaskHeader& header,
            const CompleteBitflagSeq& flag_seq);
    //}}}

    //{{{ Bitset

    /**
     * @brief Build CommonBitfield instance.
     *
     * @param [in] position Bitfield starting position bit.
     * @param [in] flags Bitfield flags: unused. No flags apply. It must be 0.
     * @param [in] bitcount Bitfield number of bits. IDL v4.2 Clause 7.4.13.4.3.2 The first one (positive_int_const) is
     *                 the number of bits that can be stored (its [bitfield] size). The maximum value is 64.
     * @param [in] holder_type Type used to manipulate the bitfield. IDL v4.2 Clause 7.4.13.4.3.2 The second optional one
     *                    (destination_type) specifies the type that will be used to manipulate the bit field as a
     *                    whole. This type can be boolean, octet or any integer type either signed or unsigned.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given position is not consistent.
     *              2. Any flag is set.
     *              3. Given bitcount is not consistent.
     *              4. Given holder_type is not consistent.
     * @return const CommonBitfield instance.
     */
    FASTDDS_EXPORTED_API static const CommonBitfield build_common_bitfield(
            uint16_t position,
            BitsetMemberFlag flags,
            uint8_t bitcount,
            TypeKind holder_type);

    /**
     * @brief Build CompleteBitfield instance.
     *
     * @param [in] common CommonBitfield.
     * @param [in] detail CompleteMemberDetail.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given CommonBitfield is inconsistent (only Debug build mode).
     *              2. Give CompleteMemberDetail is inconsistent (only Debug build mode).
     *              3. Non-applicable builtin annotations are applied.
     * @return const CompleteBitfield instance.
     */
    FASTDDS_EXPORTED_API static const CompleteBitfield build_complete_bitfield(
            const CommonBitfield& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add complete bitfield to the sequence.
     *
     * @param [in,out] sequence Sequence to be modified.
     * @param [in] bitfield CompleteBitfield to be added.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Given bitfield is inconsistent (only in Debug build mode).
     *              2. There is another bitfield with the same name and/or the same position.
     */
    FASTDDS_EXPORTED_API static void add_complete_bitfield(
            CompleteBitfieldSeq& sequence,
            const CompleteBitfield& bitfield);

    /**
     * MinimalBitfield constructed from CompleteBitfield.
     */

    /**
     * @brief Build CompleteBitsetHeader instance.
     *
     * @param [in] detail CompleteTypeDetail
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            inconsistent (only in Debug build mode).
     * @return const CompleteBitsetHeader instance.
     */
    FASTDDS_EXPORTED_API static const CompleteBitsetHeader build_complete_bitset_header(
            const CompleteTypeDetail& detail);

    /**
     * MinimalBitsetHeader constructed from CompleteBitsetHeader.
     */

    /**
     * @brief Build CompleteBitsetType instance.
     *
     * @param [in] bitset_flags Bitset flags: unused. No flags apply. It must be 0.
     * @param [in] header CompleteBitsetHeader.
     * @param [in] field_seq Sequence of complete bitfields.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if:
     *              1. Any given flag is set.
     *              2. Given header is inconsistent (only in Debug build mode).
     *              3. Given bitfield sequence is inconsistent (only in Debug build mode).
     * @return const CompleteBitsetType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteBitsetType build_complete_bitset_type(
            BitsetTypeFlag bitset_flags,
            const CompleteBitsetHeader& header,
            const CompleteBitfieldSeq& field_seq);

    /**
     * MinimalBitsetType constructed from CompleteBitsetType.
     */
    //}}}

    //{{{ Type Object

    /**
     * @brief Build CompleteExtendedType instance. (empty. Available for future extension)
     *
     * @return const CompleteExtendedType instance.
     */
    FASTDDS_EXPORTED_API static const CompleteExtendedType build_complete_extended_type();

    /**
     * @brief Register alias TypeObject into TypeObjectRegistry.
     *        CompleteAliasType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] alias_type CompleteAliasType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteAliasType just registered and the
     * generated MinimalAliasType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_alias_type_object(
            const CompleteAliasType& alias_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register annotation TypeObject into TypeObjectRegistry.
     *        CompleteAnnotationType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] annotation_type CompleteAnnotationType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteAnnotationType just registered and the
     * generated MinimalAnnotationType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_annotation_type_object(
            const CompleteAnnotationType& annotation_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register structure TypeObject into TypeObjectRegistry.
     *        CompleteStructType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] struct_type CompleteStructType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteStructType just registered and the
     * generated MinimalStructType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_struct_type_object(
            const CompleteStructType& struct_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register union TypeObject into TypeObjectRegistry.
     *        CompleteUnionType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] union_type CompleteUnionType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteUnionType just registered and the
     * generated MinimalUnionType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_union_type_object(
            const CompleteUnionType& union_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register bitset TypeObject into TypeObjectRegistry.
     *        CompleteBitsetType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] bitset_type CompleteBitsetType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteBitsetType just registered and the
     * generated MinimalBitsetType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_bitset_type_object(
            const CompleteBitsetType& bitset_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register sequence TypeObject into TypeObjectRegistry.
     *        CompleteSequenceType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] sequence_type CompleteSequenceType.
     * @param [in] type_name Name to be registered in the registry.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_sequence_type_object(
            const CompleteSequenceType& sequence_type,
            const std::string& type_name);

    /**
     * @brief Register array TypeObject into TypeObjectRegistry.
     *        CompleteArrayType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] array_type CompleteArrayType.
     * @param [in] type_name Name to be registered in the registry.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_array_type_object(
            const CompleteArrayType& array_type,
            const std::string& type_name);

    /**
     * @brief Register map TypeObject into TypeObjectRegistry.
     *        CompleteMapType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] map_type CompleteMapType.
     * @param [in] type_name Name to be registered in the registry.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_map_type_object(
            const CompleteMapType& map_type,
            const std::string& type_name);

    /**
     * @brief Register enumeration TypeObject into TypeObjectRegistry.
     *        CompleteEnumeratedType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] enumerated_type CompleteEnumeratedType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteEnumeratedType just registered and the
     * generated MinimalEnumeratedType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_enumerated_type_object(
            const CompleteEnumeratedType& enumerated_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);

    /**
     * @brief Register bitmask TypeObject into TypeObjectRegistry.
     *        CompleteBitmaskType is provided and the minimal TypeObject is constructed from the complete one.
     *
     * @param [in] bitmask_type CompleteBitmaskType.
     * @param [in] type_name Name to be registered in the registry.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteBitmaskType just registered and the
     * generated MinimalBitmaskType.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given type is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with
     *                      the given type_name.
     *                      RETCODE_BAD_PARAMETER if type_name is empty.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t build_and_register_bitmask_type_object(
            const CompleteBitmaskType& bitmask_type,
            const std::string& type_name,
            TypeIdentifierPair& type_ids);
    //}}}

    //{{{ Auxiliary public methods

    /**
     * @brief Calculate the MD5 hash of the provided name.
     *
     * @param [in] name String which hash is calculated.
     * @return const NameHash Hash of the given string.
     */
    FASTDDS_EXPORTED_API static const NameHash name_hash(
            const std::string& name);

    /**
     * @brief Check TypeObject consistency.
     *
     * @param [in] type_object Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            TypeObject is not consistent.
     */
    FASTDDS_EXPORTED_API static void type_object_consistency(
            const TypeObject& type_object);

    /**
     * @brief If one of the @ref TypeIdentifier in @ref TypeIdentifierPair is minimal, returns its reference.
     *
     * @param [in] type_ids @ref TypeIdentifierPair used to retrieve the @ref TypeIdentifier.
     * @param [out] ec Returns if there was an error.
     * @return Reference to the minimal @ref TypeIdentifier.
     */
    FASTDDS_EXPORTED_API static const TypeIdentifier& retrieve_minimal_type_identifier(
            const TypeIdentifierPair& type_ids,
            bool& ec);

    /**
     * @brief If one of the @ref TypeIdentifier in @ref TypeIdentifierPair is complete, returns its reference.
     *
     * @param [in] type_ids @ref TypeIdentifierPair used to retrieve the @ref TypeIdentifier.
     * @param [out] ec Returns if there was an error.
     * @return Reference to the complete @ref TypeIdentifier.
     */
    FASTDDS_EXPORTED_API static const TypeIdentifier& retrieve_complete_type_identifier(
            const TypeIdentifierPair& type_ids,
            bool& ec);
    //}}}

private:

    friend class TypeObjectRegistry;
    friend class eprosima::fastdds::dds::DynamicTypeBuilderFactoryImpl;

    // Class with only static methods
    TypeObjectUtils() = delete;
    ~TypeObjectUtils() = delete;

protected:

    //{{{ Auxiliary methods

    /**
     * @brief Set the try construct behavior in a given MemberFlag
     *
     * @param [in,out] member_flag Bitmask to be set.
     * @param [in] try_construct_kind @ref TryConstructFailAction.
     */
    static void set_try_construct_behavior(
            MemberFlag& member_flag,
            TryConstructFailAction try_construct_kind);

    /**
     * @brief Set the TypeFlag object.
     *
     * @param [in,out] type_flag Bitmask to be set.
     * @param [in] extensibility_kind ExtensibilityKind
     * @param [in] nested nested annotation value.
     * @param [in] autoid_hash autoid annotation has HASH value.
     */
    static void set_type_flag(
            TypeFlag& type_flag,
            ExtensibilityKind extensibility_kind,
            bool nested,
            bool autoid_hash);

    /**
     * @brief Set the extensibility kind in a given TypeFlag.
     *
     * @param [in,out] type_flag Bitmask to be set.
     * @param [in] extensibility_kind ExtensibilityKind.
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
     * @param [in] type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is fully-descriptive. false otherwise.
     */
    static bool is_fully_descriptive_type_identifier(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check if a given TypeIdentifier is direct hash.
     *        XTypes v1.3 Clause 7.3.4.6.3
     *        These are HASH TypeIdentifiers with discriminator EK_MINIMAL, EK_COMPLETE or
     *        TI_STRONGLY_CONNECTED_COMPONENT.
     *
     * @param [in] type_identifier TypeIdentifier to check.
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
     * @param [in] type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is indirect hash. false otherwise.
     */
    static bool is_indirect_hash_type_identifier(
            const TypeIdentifier& type_identifier);
    //}}}

    //{{{ Consistency methods (Debug)

    /**
     * TypeObjectHashId is always consistent. Default constructor already sets the discriminator to one valid value.
     * Union setters prevent changing the discriminator value without setting the corresponding union member.
     */

    /**
     * @brief Check LBound consistency: must be greater than 255.
     *
     * @param [in] bound LBound to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given LBound is not
     *            consistent.
     */
    static void l_bound_consistency(
            LBound bound);

    /**
     * @brief Check that the array_bound_seq is consistent: non-empty.
     *
     * @tparam T Either SBoundSeq or LBoundSeq
     * @param [in] array Sequence to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given array is not
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
        for (auto bound : array)
        {
            if (INVALID_LBOUND == bound)
            {
                throw InvalidArgumentError("bound parameter must be greater than 0");
            }
        }
    }

    /**
     * @brief Check LBoundSeq consistency.
     *
     * @param [in] bound_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given LBoundSeq is
     *            not consistent.
     */
    static void l_bound_seq_consistency(
            const LBoundSeq& bound_seq);

    /**
     * @brief Check CollectionElementFlag consistency.
     *
     * @param [in] collection_element_flag CollectionElementFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CollectionElementFlag
     *            is not consistent.
     */
    static void collection_element_flag_consistency(
            CollectionElementFlag collection_element_flag);

    /**
     * @brief Check StructMemberFlag consistency: MemberFlag consistency (try construct annotation).
     *        XTypes v1.3 Clause 7.2.2.4.4.4.8 Key members shall never be optional, and they shall always have their
     *        "must understand" attribute set to true.
     *
     * @param [in] member_flags MemberFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given StructMemberFlag is not
     *            consistent.
     */
    static void struct_member_flag_consistency(
            StructMemberFlag member_flags);

    /**
     * @brief Check UnionMemberFlag consistency.
     *
     * @param [in] union_member_flag UnionMemberFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given UnionMemberFlag
     *            is not consistent.
     */
    static void union_member_flag_consistency(
            UnionMemberFlag union_member_flag);

    /**
     * @brief Check UnionDiscriminatorFlag consistency.
     *
     * @param [in] union_discriminator_flag UnionDiscriminatorFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given UnionDiscriminatorFlag
     *            is not consistent.
     */
    static void union_discriminator_flag_consistency(
            UnionDiscriminatorFlag union_discriminator_flag);

    /**
     * @brief Check EnumeratedLiteralFlag consistency: any flag different from default are not set.
     *
     * @param [in] enumerated_literal_flag Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given EnumeratedLiteralFlag
     *            is not consistent.
     */
    static void enumerated_literal_flag_consistency(
            EnumeratedLiteralFlag enumerated_literal_flag);

    /**
     * @brief Check TypeFlag consistency: exactly one extensibility flag is set.
     *
     * @param [in] type_flag Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeFlag
     *            is not consistent.
     */
    static void type_flag_consistency(
            TypeFlag type_flag);

    /**
     * @brief Check empty flags consistency.
     *
     * @tparam T Either MemberFlag or TypeFlag.
     * @param [in] flags Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            flags are not consistent: empty (0).
     */
    template<typename T>
    static void empty_flags_consistency(
            T flags)
    {
        if (flags != 0)
        {
            throw InvalidArgumentError("Flags should be empty. No flags apply");
        }
    }

    /**
     * @brief Check EquivalenceKind consistency.
     *
     * @param [in] equiv_kind Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given EquivalenceKind
     *            is not consistent.
     */
    static void equivalence_kind_consistency(
            EquivalenceKind equiv_kind);

    /**
     * @brief Check PlainCollectionHeader consistency:
     *          - CollectionElementFlag consistent
     *          - Consistent EquivalenceKind
     *
     * @param [in] header PlainCollectionHeader to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainCollectionHeader
     *            is not consistent.
     */
    static void plain_collection_header_consistency(
            const PlainCollectionHeader& header);

    /**
     * @brief Check consistency between a given PlainCollectionHeader and the related TypeIdentifier:
     *        1. TypeIdentifier initialized
     *        2. Consistency of EquivalenceKinds
     *
     * @param [in] header PlainCollectionHeader to be checked.
     * @param [in] element_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given parameters are not
     *            consistent.
     */
    static void plain_collection_type_identifier_header_consistency(
            const PlainCollectionHeader& header,
            const TypeIdentifier& element_identifier);

    /**
     * @brief Retrieves the equivalence kind of a component within a map.
     *
     * @param [in] identifier TypeIdentifier of the component to be checked.
     * @return EK_COMPLETE if the component equivalence kind is EK_COMPLETE.
     * @return EK_MINIMAL if the component equivalence kind is EK_MINIMAL.
     * @return EK_BOTH if the component equivalence kind is EK_BOTH.
     * @return TK_NONE if the component type is invalid.
     */
    static EquivalenceKind get_map_component_equiv_kind_for_consistency(
            const TypeIdentifier& identifier);

    /**
     * @brief Check consistency between a given PlainCollectionHeader of a map and the related TypeIdentifier:
     *        1. Key TypeIdentifier is valid
     *        2. TypeIdentifier initialized
     *        3. Consistency of EquivalenceKinds
     *
     * @param [in] header PlainCollectionHeader of the map to be checked.
     * @param [in] type_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given parameters are not
     *            consistent.
     */
    static void plain_map_type_components_consistency(
            const PlainCollectionHeader& header,
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check map key_identifier consistency.
     *        XTypes v1.3 Clause 7.2.2.4.3: Implementers of this specification need only support key elements of signed
     *        and unsigned integer types and of narrow and wide string types.
     *        In Debug build mode, this method also checks that the string/wstring bound is consistent.
     *
     * @param [in] key_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier is not
     *            consistent.
     */
    static void map_key_type_identifier_consistency(
            const TypeIdentifier& key_identifier);

    /**
     * @brief Check StringLTypeDefn consistency.
     *
     * @param [in] string Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given StringLTypeDefn is not
     *            consistent.
     */
    static void string_ldefn_consistency(
            const StringLTypeDefn& string);

    /**
     * @brief Check PlainSequenceSElemDefn consistency.
     *
     * @param [in] plain_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainSequenceSElemDefn
     *            is not consistent.
     */
    static void seq_sdefn_consistency(
            const PlainSequenceSElemDefn& plain_seq);

    /**
     * @brief Check PlainSequenceLElemDefn consistency.
     *
     * @param [in] plain_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainSequenceLElemDefn
     *            is not consistent.
     */
    static void seq_ldefn_consistency(
            const PlainSequenceLElemDefn& plain_seq);

    /**
     * @brief Check PlainArraySElemDefn consistency.
     *
     * @param [in] plain_array Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainArraySElemDefn is
     *            not consistent.
     */
    static void array_sdefn_consistency(
            const PlainArraySElemDefn& plain_array);

    /**
     * @brief Check PlainArrayLElemDefn consistency.
     *
     * @param [in] plain_array Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainArrayLElemDefn is
     *            not consistent.
     */
    static void array_ldefn_consistency(
            const PlainArrayLElemDefn& plain_array);

    /**
     * @brief Check PlainMapSTypeDefn consistency.
     *
     * @param [in] plain_map Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainMapSTypeDefn is
     *            not consistent.
     */
    static void map_sdefn_consistency(
            const PlainMapSTypeDefn& plain_map);

    /**
     * @brief Check PlainMapLTypeDefn consistency.
     *
     * @param [in] plain_map Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given PlainMapLTypeDefn is
     *            not consistent.
     */
    static void map_ldefn_consistency(
            const PlainMapLTypeDefn& plain_map);

    /**
     * @brief Check direct hash TypeIdentifier consistency.
     *
     * @param [in] type_id Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier is
     *            not consistent.
     */
    static void direct_hash_type_identifier_consistency(
            const TypeIdentifier& type_id);

    /**
     * @brief Check TypeIdentifier consistency.
     *
     * @param [in] type_identifier Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier is
     *            not consistent.
     */
    static void type_identifier_consistency(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check AppliedAnnotationParameterSeq consistency.
     *
     * @param [in] applied_annotation_parameter_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            AppliedAnnotationParameterSeq is not consistent.
     */
    static void applied_annotation_parameter_seq_consistency(
            const AppliedAnnotationParameterSeq& applied_annotation_parameter_seq);

    /**
     * @brief Check AppliedAnnotation TypeIdentifier consistency.
     *
     * @param [in] annotation_type_id Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            AppliedAnnotation TypeIdentifier is not consistent.
     */
    static void applied_annotation_type_identifier_consistency(
            const TypeIdentifier& annotation_type_id);

    /**
     * @brief Check AppliedAnnotation consistency.
     *
     * @param [in] applied_annotation Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given AppliedAnnotation is
     *            not consistent.
     */
    static void applied_annotation_consistency(
            const AppliedAnnotation& applied_annotation);

    /**
     * @brief Check AppliedAnnotationSeq consistency.
     *
     * @param [in] applied_annotation_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given AppliedAnnotationSeq is
     *            not consistent.
     */
    static void applied_annotation_seq_consistency(
            const AppliedAnnotationSeq& applied_annotation_seq);

    /**
     * @brief Check AppliedVerbatimAnnotation consistency.
     *
     * @param [in] applied_verbatim_annotation Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     * AppliedVerbatimAnnotation is not consistent.
     */
    static void applied_verbatim_annotation_consistency(
            const AppliedVerbatimAnnotation& applied_verbatim_annotation);

    /**
     * @brief Check CommonStructMember consistency.
     *
     * @param [in] common_struct_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CommonStructMember is
     *            not consistent.
     */
    static void common_struct_member_consistency(
            const CommonStructMember& common_struct_member);

    /**
     * @brief Check CompleteMemberDetail consistency.
     *
     * @param [in] complete_member_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteMemberDetail is
     *            not consistent.
     */
    static void complete_member_detail_consistency(
            const CompleteMemberDetail& complete_member_detail);

    /**
     * @brief Check cross-consistency between CommonStructMember and CompleteMemberDetail.
     *
     * @param [in] common_struct_member CommonStructMember to be checked.
     * @param [in] complete_member_detail CompleteMemberDetail to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the hashid builtin applied
     *            annotation is set and inconsistent with the member id.
     */
    static void common_struct_member_and_complete_member_detail_consistency(
            const CommonStructMember& common_struct_member,
            const CompleteMemberDetail& complete_member_detail);

    /**
     * @brief Check consistency between a string value and the MemberId (algorithm XTypes v1.3 Clause 7.3.1.2.1.1)
     *
     * @param [in] member_id MemberId to be checked.
     * @param [in] string_value String provided with either hashid annotation or the member name.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given data is inconsistent.
     */
    static void string_member_id_consistency(
            MemberId member_id,
            const std::string& string_value);

    /**
     * @brief Check CompleteStructMember consistency.
     *
     * @param [in] complete_struct_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteStructMember is
     *            not consistent.
     */
    static void complete_struct_member_consistency(
            const CompleteStructMember& complete_struct_member);

    /**
     * @brief Check CompleteStructMemberSeq consistency.
     *
     * @param [in] complete_struct_member_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteStructMemberSeq
     *            is not consistent.
     */
    static void complete_struct_member_seq_consistency(
            const CompleteStructMemberSeq& complete_struct_member_seq);

    /**
     * @brief Check AppliedBuiltinTypeAnnotations consistency.
     *
     * @param [in] applied_builtin_type_annotations Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            AppliedBuiltinTypeAnnotations is not consistent.
     */
    static void applied_builtin_type_annotations_consistency(
            const AppliedBuiltinTypeAnnotations& applied_builtin_type_annotations);

    /**
     * @brief Check CompleteTypeDetail consistency.
     *
     * @param [in] complete_type_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            not consistent.
     */
    static void complete_type_detail_consistency(
            const CompleteTypeDetail& complete_type_detail);

    /**
     * @brief Check CompleteStructHeader base_type TypeIdentifier consistency.
     *
     * @param [in] base_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier is
     *            not consistent.
     */
    static void structure_base_type_consistency(
            const TypeIdentifier& base_type);

    /**
     * @brief Check CompleteStructHeader consistency.
     *
     * @param [in] complete_struct_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteStructHeader
     *            is not consistent.
     */
    static void complete_struct_header_consistency(
            const CompleteStructHeader& complete_struct_header);

    /**
     * @brief Check CompleteStructType consistency.
     *
     * @param [in] complete_struct_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteStructType
     *            is not consistent.
     */
    static void complete_struct_type_consistency(
            const CompleteStructType& complete_struct_type);

    /**
     * @brief Check MinimalStructType consistency.
     *
     * @param [in] minimal_struct_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given MinimalStructType
     *            is not consistent.
     */
    static void minimal_struct_type_consistency(
            const MinimalStructType& minimal_struct_type);

    /**
     * @brief Check UnionCaseLabelSeq consistency.
     *
     * @param [in] union_case_label_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given UnionCaseLabelSeq
     *            is not consistent.
     */
    static void union_case_label_seq_consistency(
            const UnionCaseLabelSeq& union_case_label_seq);

    /**
     * @brief Check CommonUnionMember consistency.
     *
     * @param [in] common_union_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CommonUnionMember
     *            is not consistent.
     */
    static void common_union_member_consistency(
            const CommonUnionMember& common_union_member);

    /**
     * @brief Check cross-consistency between CommonStructMember and CompleteMemberDetail.
     *
     * @param [in] common_union_member CommonStructMember to be checked.
     * @param [in] complete_member_detail CompleteMemberDetail to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the hashid builtin annotation is
     *            set and the member id is not consistent.
     */
    static void common_union_member_complete_member_detail_consistency(
            const CommonUnionMember& common_union_member,
            const CompleteMemberDetail& complete_member_detail);

    /**
     * @brief Check CompleteUnionMember consistency.
     *
     * @param [in] complete_union_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteUnionMember
     *            is not consistent.
     */
    static void complete_union_member_consistency(
            const CompleteUnionMember& complete_union_member);

    /**
     * @brief Check CompleteUnionMemberSeq consistency.
     *
     * @param [in] complete_union_member_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given CompleteUnionMemberSeq
     *            is not consistent.
     */
    static void complete_union_member_seq_consistency(
            const CompleteUnionMemberSeq& complete_union_member_seq);

    /**
     * @brief Check discriminator TypeIdentifier consistency.
     *
     * @param [in] type_id Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier
     *            is not consistent.
     */
    static void common_discriminator_member_type_identifier_consistency(
            const TypeIdentifier& type_id);

    /**
     * @brief Check CommonDiscriminatorMember consistency.
     *
     * @param [in] common_discriminator_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonDiscriminatorMember is not consistent.
     */
    static void common_discriminator_member_consistency(
            const CommonDiscriminatorMember& common_discriminator_member);

    /**
     * @brief Check CompleteDiscriminatorMember consistency.
     *
     * @param [in] complete_discriminator_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteDiscriminatorMember is not consistent.
     */
    static void complete_discriminator_member_consistency(
            const CompleteDiscriminatorMember& complete_discriminator_member);

    /**
     * @brief Check CompleteUnionHeader consistency.
     *
     * @param [in] complete_union_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteUnionHeader is not consistent.
     */
    static void complete_union_header_consistency(
            const CompleteUnionHeader& complete_union_header);

    /**
     * @brief Check CompleteUnionType consistency.
     *
     * @param [in] complete_union_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteUnionType is not consistent.
     */
    static void complete_union_type_consistency(
            const CompleteUnionType& complete_union_type);

    /**
     * @brief Check MinimalUnionType consistency.
     *
     * @param [in] minimal_union_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalUnionType is not consistent.
     */
    static void minimal_union_type_consistency(
            const MinimalUnionType& minimal_union_type);

    /**
     * @brief Check that the annotation value is of the same type as the given TypeIdentifier.
     *
     * @param type_id TypeIdentifier.
     * @param value AnnotationParameterValue.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given TypeIdentifier is not
     *            consistent with the given value.
     */
    static void common_annotation_parameter_type_identifier_default_value_consistency(
            const TypeIdentifier& type_id,
            const AnnotationParameterValue& value);

    /**
     * @brief Check CommonAnnotationParameter consistency.
     *
     * @param [in] common_annotation_parameter Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonAnnotationParameter is not consistent.
     */
    static void common_annotation_parameter_consistency(
            const CommonAnnotationParameter& common_annotation_parameter);

    /**
     * @brief Check CompleteAnnotationParameter consistency.
     *
     * @param [in] complete_annotation_parameter Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAnnotationParameter is not consistent.
     */
    static void complete_annotation_parameter_consistency(
            const CompleteAnnotationParameter& complete_annotation_parameter);

    /**
     * @brief Check CompleteAnnotationParameterSeq consistency.
     *
     * @param [in] complete_annotation_parameter_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAnnotationParameterSeq is not consistent.
     */
    static void complete_annotation_parameter_seq_consistency(
            const CompleteAnnotationParameterSeq& complete_annotation_parameter_seq);

    /**
     * @brief Check CompleteAnnotationHeader consistency.
     *
     * @param [in] complete_annotation_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAnnotationHeader is not consistent.
     */
    static void complete_annotation_header_consistency(
            const CompleteAnnotationHeader& complete_annotation_header);

    /**
     * @brief Check CompleteAnnotationType consistency.
     *
     * @param [in] complete_annotation_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAnnotationType is not consistent.
     */
    static void complete_annotation_type_consistency(
            const CompleteAnnotationType& complete_annotation_type);

    /**
     * @brief Check MinimalAnnotationType consistency.
     *
     * @param [in] minimal_annotation_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalAnnotationType is not consistent.
     */
    static void minimal_annotation_type_consistency(
            const MinimalAnnotationType& minimal_annotation_type);

    /**
     * @brief Check CommonAliasBody consistency.
     *
     * @param [in] common_alias_body Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonAliasBody is not consistent.
     */
    static void common_alias_body_consistency(
            const CommonAliasBody& common_alias_body);

    /**
     * @brief Check that hashid builtin annotation has not been set.
     *
     * @param [in] ann_builtin Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            AppliedBuiltinMemberAnnotations is not consistent.
     */
    static void hashid_builtin_annotation_not_applied_consistency(
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin);

    /**
     * @brief Check CompleteAliasBody consistency.
     *
     * @param [in] complete_alias_body Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAliasBody is not consistent.
     */
    static void complete_alias_body_consistency(
            const CompleteAliasBody& complete_alias_body);

    /**
     * @brief Check CompleteAliasHeader consistency.
     *
     * @param [in] complete_alias_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAliasHeader is not consistent.
     */
    static void complete_alias_header_consistency(
            const CompleteAliasHeader& complete_alias_header);

    /**
     * @brief Check CompleteAliasType consistency.
     *
     * @param [in] complete_alias_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteAliasType is not consistent.
     */
    static void complete_alias_type_consistency(
            const CompleteAliasType& complete_alias_type);

    /**
     * @brief Check MinimalAliasType consistency.
     *
     * @param [in] minimal_alias_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalAliasType is not consistent.
     */
    static void minimal_alias_type_consistency(
            const MinimalAliasType& minimal_alias_type);

    /**
     * @brief Check CompleteElementDetail consistency.
     *
     * @param [in] complete_element_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteElementDetail is not consistent.
     */
    static void complete_element_detail_consistency(
            const CompleteElementDetail& complete_element_detail);

    /**
     * @brief Check CommonCollectionElement consistency.
     *
     * @param [in] common_collection_element Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonCollectionElement is not consistent.
     */
    static void common_collection_element_consistency(
            const CommonCollectionElement& common_collection_element);

    /**
     * @brief Check CompleteCollectionElement consistency.
     *
     * @param [in] complete_collection_element Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteCollectionElement is not consistent.
     */
    static void complete_collection_element_consistency(
            const CompleteCollectionElement& complete_collection_element);

    /**
     * @brief Check CompleteCollectionHeader consistency.
     *
     * @param [in] complete_collection_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteCollectionHeader is not consistent.
     */
    static void complete_collection_header_consistency(
            const CompleteCollectionHeader& complete_collection_header);

    /**
     * @brief Check CompleteSequenceType consistency.
     *
     * @param [in] complete_sequence_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteSequenceType is not consistent.
     */
    static void complete_sequence_type_consistency(
            const CompleteSequenceType& complete_sequence_type);

    /**
     * @brief Check MinimalSequenceType consistency.
     *
     * @param [in] minimal_sequence_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalSequenceType is not consistent.
     */
    static void minimal_sequence_type_consistency(
            const MinimalSequenceType& minimal_sequence_type);

    /**
     * @brief Check CommonArrayHeader consistency.
     *
     * @param [in] common_array_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonArrayHeader is not consistent.
     */
    static void common_array_header_consistency(
            const CommonArrayHeader& common_array_header);

    /**
     * @brief Check CompleteArrayHeader consistency.
     *
     * @param [in] complete_array_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteArrayHeader is not consistent.
     */
    static void complete_array_header_consistency(
            const CompleteArrayHeader& complete_array_header);

    /**
     * @brief Check CompleteArrayType consistency.
     *
     * @param [in] complete_array_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteArrayType is not consistent.
     */
    static void complete_array_type_consistency(
            const CompleteArrayType& complete_array_type);

    /**
     * @brief Check MinimalArrayType consistency.
     *
     * @param [in] minimal_array_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalArrayType is not consistent.
     */
    static void minimal_array_type_consistency(
            const MinimalArrayType& minimal_array_type);

    /**
     * @brief Check CompleteMapType consistency.
     *
     * @param [in] complete_map_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteMapType is not consistent.
     */
    static void complete_map_type_consistency(
            const CompleteMapType& complete_map_type);

    /**
     * @brief Check MinimalMapType consistency.
     *
     * @param [in] minimal_map_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalMapType is not consistent.
     */
    static void minimal_map_type_consistency(
            const MinimalMapType& minimal_map_type);

    /**
     * @brief Check CommonEnumeratedLiteral consistency.
     *
     * @param [in] common_enumerated_literal Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonEnumeratedLiteral is not consistent.
     */
    static void common_enumerated_literal_consistency(
            const CommonEnumeratedLiteral& common_enumerated_literal);

    /**
     * @brief Check CompleteEnumeratedLiteral consistency.
     *
     * @param [in] complete_enumerated_literal Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteEnumeratedLiteral is not consistent.
     */
    static void complete_enumerated_literal_consistency(
            const CompleteEnumeratedLiteral& complete_enumerated_literal);

    /**
     * @brief Check CompleteEnumeratedLiteralSeq consistency.
     *
     * @param [in] complete_enumerated_literal_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteEnumeratedLiteralSeq is not consistent.
     */
    static void complete_enumerated_literal_seq_consistency(
            const CompleteEnumeratedLiteralSeq& complete_enumerated_literal_seq);

    /**
     * @brief Check enumeration BitBound consistency.
     *
     * @param [in] bit_bound Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            BitBound is not consistent.
     */
    static void enum_bit_bound_consistency(
            BitBound bit_bound);

    /**
     * @brief Check bitmask BitBound consistency.
     *
     * @param [in] bit_bound Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            BitBound is not consistent.
     */
    static void bitmask_bit_bound_consistency(
            BitBound bit_bound);

    /**
     * @brief Check CommonEnumeratedHeader consistency.
     *
     * @param [in] common_enumerated_header Instance to be checked.
     * @param [in] bitmask flag in case that the header corresponds to a Bitmask. By default is false.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonEnumeratedHeader is not consistent.
     */
    static void common_enumerated_header_consistency(
            const CommonEnumeratedHeader& common_enumerated_header,
            bool bitmask = false);

    /**
     * @brief Check CompleteEnumeratedHeader consistency.
     *
     * @param [in] complete_enumerated_header Instance to be checked.
     * @param [in] bitmask Flag in case that the header corresponds to a Bitmask. By default is false.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteEnumeratedHeader is not consistent.
     */
    static void complete_enumerated_header_consistency(
            const CompleteEnumeratedHeader& complete_enumerated_header,
            bool bitmask = false);

    /**
     * @brief Check CompleteEnumeratedType consistency.
     *
     * @param [in] complete_enumerated_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteEnumeratedType is not consistent.
     */
    static void complete_enumerated_type_consistency(
            const CompleteEnumeratedType& complete_enumerated_type);

    /**
     * @brief Check MinimalEnumeratedType consistency.
     *
     * @param [in] minimal_enumerated_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalEnumeratedType is not consistent.
     */
    static void minimal_enumerated_type_consistency(
            const MinimalEnumeratedType& minimal_enumerated_type);

    /**
     * @brief Check bitflag position consistency.
     *
     * @param [in] position Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            bitflag position is not consistent.
     */
    static void bit_position_consistency(
            uint16_t position);

    /**
     * @brief Check CommonBitflag consistency.
     *
     * @param [in] common_bitflag Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonBitflag is not consistent.
     */
    static void common_bitflag_consistency(
            const CommonBitflag& common_bitflag);

    /**
     * @brief Check CompleteBitflag consistency.
     *
     * @param [in] complete_bitflag Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitflag is not consistent.
     */
    static void complete_bitflag_consistency(
            const CompleteBitflag& complete_bitflag);

    /**
     * @brief Check CompleteBitflagSeq consistency.
     *
     * @param [in] complete_bitflag_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitflagSeq is not consistent.
     */
    static void complete_bitflag_seq_consistency(
            const CompleteBitflagSeq& complete_bitflag_seq);

    /**
     * @brief Check CompleteBitmaskType consistency.
     *
     * @param [in] complete_bitmask_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitmaskType is not consistent.
     */
    static void complete_bitmask_type_consistency(
            const CompleteBitmaskType& complete_bitmask_type);

    /**
     * @brief Check MinimalBitmaskType consistency.
     *
     * @param [in] minimal_bitmask_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalBitmaskType is not consistent.
     */
    static void minimal_bitmask_type_consistency(
            const MinimalBitmaskType& minimal_bitmask_type);

    /**
     * @brief Check consistency between the holder type and the bitcount.
     *
     * @param [in] holder_type TypeKind of the bitfield holder type.
     * @param [in] bitcount Bitfield number of bits.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given data is inconsistent.
     */
    static void bitfield_holder_type_consistency(
            TypeKind holder_type,
            uint8_t bitcount);

    /**
     * @brief Check CommonBitfield consistency.
     *
     * @param [in] common_bitfield Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CommonBitfield is not consistent.
     */
    static void common_bitfield_consistency(
            const CommonBitfield& common_bitfield);

    /**
     * @brief Check CompleteBitfield consistency.
     *
     * @param [in] complete_bitfield Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitfield is not consistent.
     */
    static void complete_bitfield_consistency(
            const CompleteBitfield& complete_bitfield);

    /**
     * @brief Check CompleteBitfieldSeq consistency.
     *
     * @param [in] complete_bitfield_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitfieldSeq is not consistent.
     */
    static void complete_bitfield_seq_consistency(
            const CompleteBitfieldSeq& complete_bitfield_seq);

    /**
     * @brief Check CompleteBitsetHeader consistency.
     *
     * @param [in] complete_bitset_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitsetHeader is not consistent.
     */
    static void complete_bitset_header_consistency(
            const CompleteBitsetHeader& complete_bitset_header);

    /**
     * @brief Check CompleteBitsetType consistency.
     *
     * @param [in] complete_bitset_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteBitsetType is not consistent.
     */
    static void complete_bitset_type_consistency(
            const CompleteBitsetType& complete_bitset_type);

    /**
     * @brief Check MinimalBitsetType consistency.
     *
     * @param [in] minimal_bitset_type Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalBitsetType is not consistent.
     */
    static void minimal_bitset_type_consistency(
            const MinimalBitsetType& minimal_bitset_type);

    /**
     * @brief Check CompleteTypeObject consistency.
     *
     * @param [in] complete_type_object Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            CompleteTypeObject is not consistent.
     */
    static void complete_type_object_consistency(
            const CompleteTypeObject& complete_type_object);

    /**
     * @brief Check MinimalTypeObject consistency.
     *
     * @param [in] minimal_type_object Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes::InvalidArgumentError exception if the given
     *            MinimalTypeObject is not consistent.
     */
    static void minimal_type_object_consistency(
            const MinimalTypeObject& minimal_type_object);
    //}}}
};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP
