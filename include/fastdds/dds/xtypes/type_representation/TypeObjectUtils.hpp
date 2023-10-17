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

#include <fastcdr/cdr/fixed_size_string.hpp>
#include <fastcdr/xcdr/optional.hpp>

#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

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
     * @param[in] extensibility_kind extensibility annotation value.
     * @param[in] nested nested annotation value.
     * @param[in] autoid_hash autoid annotation has HASH value.
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
     * @param[in] equiv_kind EquivalenceKind: EK_MINIMAL/EK_COMPLETE/EK_BOTH
     * @param[in] element_flags CollectionElementFlags to be set. This element must be constructed with the corresponding
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] bound Sequence bound.
     * @param[in] element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. The given bound is 0.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] bound Sequence bound.
     * @param[in] element_identifier Sequence element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Bound lower than 256.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] array_bound_seq Bounds for the array dimensions.
     * @param[in] element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              3. Inconsistent header (only in Debug build mode).
     *              4. Inconsistent element_identifier (only in Debug build mode).
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] array_bound_seq Bounds for the array dimensions.
     * @param[in] element_identifier Array element TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
     *              1. Any given bound in array_bound_seq is 0.
     *              2. There is no dimension with a bound greater than 255.
     *              3. The given TypeIdentifier EquivalenceKind is not consistent with the one contained in the header.
     *              4. Inconsistent header (only in Debug build mode).
     *              5. Inconsistent element_identifier (only in Debug build mode).
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] bound Map bound.
     * @param[in] element_identifier Map element TypeIdentifier.
     * @param[in] key_flags Flags applying to map key.
     * @param[in] key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
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
     * @param[in] header PlainCollectionHeader to be set.
     * @param[in] bound Map bound.
     * @param[in] element_identifier Map element TypeIdentifier.
     * @param[in] key_flags Flags applying to map key.
     * @param[in] key_identifier Map key TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception
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
    RTPS_DllAPI static const PlainMapLTypeDefn build_plain_map_l_type_defn(
            const PlainCollectionHeader& header,
            const LBound bound,
            const TypeIdentifier& element_identifier,
            const CollectionElementFlag key_flags,
            const TypeIdentifier& key_identifier);

    /**
     * @brief Build StronglyConnectedComponentId instance.
     *
     * @param[in] sc_component_id Strongly Connected Component (SCC) ID.
     * @param[in] scc_length Number of components within SCC.
     * @param[in] scc_index Identify specific component within SCC.
     * @return const StronglyConnectedComponentId instance.
     */
    RTPS_DllAPI static const StronglyConnectedComponentId build_strongly_connected_component_id(
            const TypeObjectHashId& sc_component_id,
            long scc_length,
            long scc_index);

    /**
     * @brief Build ExtendedTypeDefn instance (empty. Available for future extension).
     *
     * @return const ExtendedTypeDefn instance.
     */
    RTPS_DllAPI static const ExtendedTypeDefn build_extended_type_defn();

    /*************** Register Indirect Hash TypeIdentifiers ***************************/
    /**
     * Primitive types are registered when TypeObjectRegistry is instantiated.
     */

    /**
     * @brief Register small string/wstring TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] string StringSTypeDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_s_string_type_identifier(
            const StringSTypeDefn& string,
            const std::string& type_name);

    /**
     * @brief Register large string/wstring TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] string StringLTypeDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_l_string_type_identifier(
            const StringLTypeDefn& string,
            const std::string& type_name);

    /**
     * @brief Register small sequence TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_seq PlainSequenceSElemDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_s_sequence_type_identifier(
            const PlainSequenceSElemDefn& plain_seq,
            const std::string& type_name);

    /**
     * @brief Register large sequence TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_seq PlainSequenceLElemDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_l_sequence_type_identifier(
            const PlainSequenceLElemDefn& plain_seq,
            const std::string& type_name);

    /**
     * @brief Register small array TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_array PlainArraySElemDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_s_array_type_identifier(
            const PlainArraySElemDefn& plain_array,
            const std::string& type_name);

    /**
     * @brief Register large array TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_array PlainArrayLElemDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_l_array_type_identifier(
            const PlainArrayLElemDefn& plain_array,
            const std::string& type_name);

    /**
     * @brief Register small map TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_map PlainMapSTypeDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_s_map_type_identifier(
            const PlainMapSTypeDefn& plain_map,
            const std::string& type_name);

    /**
     * @brief Register large map TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] plain_map PlainMapLTypeDefn union member to set.
     * @param[in] type_name Type name to be registered.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given member is inconsistent
     *            (only in Debug build mode).
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_l_map_type_identifier(
            const PlainMapLTypeDefn& plain_map,
            const std::string& type_name);

    /**
     * @brief Register StronglyConnectedComponent TypeIdentifier into TypeObjectRegistry.
     *
     * @param[in] scc StronglyConnectedComponent union member to set.
     * @param[in] type_name Type name to be registered.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is direct hash TypeIdentifier.
     */
    RTPS_DllAPI static ReturnCode_t build_and_register_scc_type_identifier(
            const StronglyConnectedComponentId& scc,
            const std::string& type_name);

    /*************** Annotation usage ***************************/
    /**
     * @brief Build ExtendedAnnotationParameterValue instance (empty. Available for future extension).
     *
     * @return const ExtendedAnnotationParameterValue instance.
     */
    RTPS_DllAPI static const ExtendedAnnotationParameterValue build_extended_annotation_parameter_value();

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] boolean_value Boolean value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            bool boolean_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] byte_value Byte value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value_byte(
            uint8_t byte_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] int8_value Int8 value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            int8_t int8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] uint8_value Unsigned int8 value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            uint8_t uint8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] int16_value Short value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            int16_t int16_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] uint16_value Unsigned short value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            uint16_t uint16_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] int32_value Long value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            int32_t int32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] uint32_value Unsigned long value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            uint32_t uint32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] int64_value Long long value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            int64_t int64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] uint64_value Unsigned long long value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            uint64_t uint64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] float32_value Float value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            float float32_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] float64_value Double value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            double float64_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] float128_value Long double value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            long double float128_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] char_value Char value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            char char_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] wchar_value Wide char value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            wchar_t wchar_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] enumerated_value Enumerated value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value_enum(
            int32_t enumerated_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] string8_value String value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            const eprosima::fastcdr::fixed_string<128>& string8_value);

    /**
     * @brief Build AnnotationParameterValue instance.
     *
     * @param[in] string16_value Wide string value to set in the union.
     * @return const AnnotationParameterValue instance. 
     */
    RTPS_DllAPI static const AnnotationParameterValue build_annotation_parameter_value(
            const std::wstring& string16_value);

    /**
     * @brief Build AppliedAnnotationParameter instance.
     *
     * @param[in] paramname_hash Parameter name hash.
     * @param[in] value Parameter value.
     * @return const AppliedAnnotationParameter instance.
     */
    RTPS_DllAPI static const AppliedAnnotationParameter build_applied_annotation_parameter(
            const NameHash& paramname_hash,
            const AnnotationParameterValue& value);

    /**
     * @brief Add AppliedAnnotationParameter to the sequence.
     *
     * @param[in out] param_seq AppliedAnnotationParameter sequence to be modified.
     * @param[in] param AppliedAnnotationParameter to be added.
     */
    RTPS_DllAPI static void add_applied_annotation_parameter(
            AppliedAnnotationParameterSeq& param_seq,
            const AppliedAnnotationParameter& param);

    /**
     * @brief Build AppliedAnnotation instance.
     *
     * @param[in] annotation_typeid Annotation TypeIdentifier.
     * @param[in] param_seq Annotation parameters.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given annotation_typeid
     *            TypeIdentifier is not a direct HASH (only in Debug build mode).
     * @return const AppliedAnnotation instance.
     */
    RTPS_DllAPI static const AppliedAnnotation build_applied_annotation(
            const TypeIdentifier& annotation_typeid,
            const eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>& param_seq);

    /**
     * @brief Build AppliedVerbatimAnnotation instance.
     *
     * @param[in] placement Verbatim annotation placement parameter.
     * @param[in] language Verbatim annotation language parameter.
     * @param[in] text Verbatim annotation text parameter.
     * @return const AppliedVerbatimAnnotation instance.
     */
    RTPS_DllAPI static const AppliedVerbatimAnnotation build_applied_verbatim_annotation(
            PlacementKindValue placement,
            const eprosima::fastcdr::fixed_string<32>& language,
            const std::string& text);

    /*************** Aggregate types: ***************************/
    /**
     * @brief Build AppliedBuiltinMemberAnnotations instance.
     *
     * @param[in] unit Unit annotation value.
     * @param[in] min Min annotation value.
     * @param[in] max Max annotation value.
     * @param[in] hash_id Hashid annotation value.
     * @return const AppliedBuiltinMemberAnnotations instance.
     */
    RTPS_DllAPI static const AppliedBuiltinMemberAnnotations build_applied_builtin_member_annotations(
            const eprosima::fastcdr::optional<std::string>& unit,
            const eprosima::fastcdr::optional<AnnotationParameterValue>& min,
            const eprosima::fastcdr::optional<AnnotationParameterValue>& max,
            const eprosima::fastcdr::optional<std::string>& hash_id);

    /**
     * @brief Build CommonStructMember instance.
     *
     * @param[in] member_id Member identifier.
     * @param[in] member_flags Member flags: optional, must_understand, key, and external.
     * @param[in] member_type_id Member TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception:
     *              1. The given flags are not consistent (only in Debug build mode).
     *              2. The given TypeIdentifier is not consistent (only in Debug build mode).
     * @return const CommonStructMember instance.
     */
    RTPS_DllAPI static const CommonStructMember build_common_struct_member(
            MemberId member_id,
            StructMemberFlag member_flags,
            const TypeIdentifier& member_type_id);

    /**
     * @brief Add AppliedAnnotation to the sequence.
     *
     * @param[in out] ann_custom_seq AppliedAnnotation sequence to be modified.
     * @param[in] ann_custom AppliedAnnotation to be added.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given AppliedAnnotation is
     *            not consistent.
     */
    RTPS_DllAPI static void add_applied_annotation_parameter(
            AppliedAnnotationSeq& ann_custom_seq,
            const AppliedAnnotation& ann_custom);

    /**
     * @brief Build CompleteMemberDetail instance.
     *
     * @param[in] name Member name.
     * @param[in] ann_builtin Member builtin annotations.
     * @param[in] ann_custom Member custom annotations.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the AppliedAnnotationSeq is not
     *            consistent (only Debug build mode).
     * @return const CompleteMemberDetail instance.
     */
    RTPS_DllAPI static const CompleteMemberDetail build_complete_member_detail(
            const MemberName& name,
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * MinimalMemberDetail constructed from CompleteMemberDetail
     */

    /**
     * @brief Build CompleteStructMember instance.
     *
     * @param[in] common CommonStructMember to be set.
     * @param[in] detail CompleteMemberDetail to be set.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given CommonStructMember is inconsistent (only Debug build mode).
     *              2. Given CompleteMemberDetail is inconsistent (only Debug build mode).
     * @return const CompleteMemberDetail instance.
     */
    RTPS_DllAPI static const CompleteStructMember build_complete_struct_member(
            const CommonStructMember& common,
            const CompleteMemberDetail& detail);

    /**
     * MinimalStructMember constructed from CompleteStructMember
     */

    /**
     * @brief Build AppliedBuiltinTypeAnnotations instance.
     *
     * @param[in] verbatim AppliedVerbatimAnnotation to be set.
     * @return const AppliedBuiltinTypeAnnotations instance.
     */
    RTPS_DllAPI static const AppliedBuiltinTypeAnnotations build_applied_builtin_type_annotations(
            const eprosima::fastcdr::optional<AppliedVerbatimAnnotation>& verbatim);

    /**
     * MinimalTypeDetail constructed from CompleteTypeDetail: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteTypeDetail instance.
     *
     * @param[in] ann_builtin Verbatim annotation.
     * @param[in] ann_custom Applied annotations.
     * @param[in] type_name Name of the type.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if any applied annotation is not
     *            consistent (only Debug build mode).
     * @return const CompleteTypeDetail instance.
     */
    RTPS_DllAPI static const CompleteTypeDetail build_complete_type_detail(
            const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom,
            const QualifiedTypeName& type_name);

    /**
     * @brief Build CompleteStructHeader instance.
     *
     * @param[in] base_type TypeIdentifier of the parent structure (inheritance).
     * @param[in] detail CompleteTypeDetail.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given TypeIdentifier is not consistent (direct HASH TypeIdentifier).
     *              2. Given CompleteTypeDetail is not consistent.
     * @return const CompleteStructHeader instance.
     */
    RTPS_DllAPI static const CompleteStructHeader build_complete_struct_header(
            const TypeIdentifier& base_type,
            const CompleteTypeDetail& detail);

    /**
     * MinimalStructHeader constructed from CompleteStructHeader.
     */

    /**
     * @brief Add CompleteStructMember to the sequence.
     *
     * @param[in out] member_seq CompleteStructMember sequence to be modified.
     * @param[in] ann_custom CompleteStructMember to be added.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given CompleteStructMember
     *            is not consistent.
     */
    RTPS_DllAPI static void add_complete_struct_member(
            CompleteStructMemberSeq& member_seq,
            const CompleteStructMember& member);

    /**
     * @brief Build CompleteStructType instance.
     *
     * @param[in] struct_flags StructTypeFlags.
     * @param[in] header CompleteStructHeader.
     * @param[in] member_seq Sequence of CompleteStructMembers.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given StructTypeFlag is not consistent (only in Debug build mode).
     *              2. Given CompleteStructHeader is not consistent (only in Debug build mode).
     *              3. Given CompleteStructMemberSeq is not consistent (only in Debug build mode).
     * @return const CompleteStructType instance. 
     */
    RTPS_DllAPI static const CompleteStructType build_complete_struct_type(
            StructTypeFlag struct_flags,
            const CompleteStructHeader& header,
            const CompleteStructMemberSeq& member_seq);

    /**
     * MinimalStructType constructed from CompleteStructType.
     */

    /*************** Union: *********************************************/

    /**
     * @brief Add label to the union case label sequence.
     *
     * @param[in out] label_seq Sequence to be modified.
     * @param[in] label Label to be added.
     */
    RTPS_DllAPI static void add_union_case_label(
            UnionCaseLabelSeq& label_seq,
            int32_t label);

    /**
     * @brief Build CommonUnionMember instance.
     *
     * @param[in] member_id Member identifier.
     * @param[in] member_flags Member flags.
     * @param[in] type_id Member TypeIdentifier.
     * @param[in] label_seq Member applicable case labels.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given UnionMemberFlags are not consistent (only Debug build mode).
     *              2. Given TypeIdentifier is not consistent (only Debug build mode).
     * @return const CommonUnionMember instance.
     */
    RTPS_DllAPI static const CommonUnionMember build_common_union_member(
            MemberId member_id,
            UnionMemberFlag member_flags,
            const TypeIdentifier& type_id,
            const UnionCaseLabelSeq& label_seq);

    /**
     * @brief Build CompleteUnionMember instance.
     *
     * @param[in] common CommonUnionMember.
     * @param[in] detail CompleteMemberDetail.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given CommonUnionMember is not consistent (only in Debug build mode).
     *              2. Given CompleteMemberDetail is not consistent (only in Debug build mode).
     * @return const CompleteUnionMember instance.
     */
    RTPS_DllAPI static const CompleteUnionMember build_complete_union_member(
            const CommonUnionMember& common,
            const CompleteMemberDetail& detail);

    /**
     * @brief Add CompleteUnionMember to sequence.
     *
     * @param[in out] complete_union_member_seq Sequence to be modified.
     * @param[in] member Complete union member to be added.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given CompleteUnionMember is
     *            not consistent (only in Debug build mode).
     */
    RTPS_DllAPI static void add_complete_union_member(
            CompleteUnionMemberSeq& complete_union_member_seq,
            const CompleteUnionMember& member);

    /**
     * MinimalUnionMember constructed from CompleteUnionMember.
     */

    /**
     * @brief Build CommonDiscriminatorMember instance.
     *
     * @param[in] member_flags Discriminator flags.
     * @param[in] type_id Discriminator TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given discriminator flags are inconsistent (only in Debug build mode).
     *              2. Given TypeIdentifier is not consistent (only in Debug build mode).
     *                 XTypes v1.3 Clause 7.2.2.4.4.3 The discriminator of a union must be one of the following types:
     *                 Boolean, Byte, Char8, Char16, Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64, Uint64, any
     *                 enumerated type, any alias type that resolves, directly or indirectly, to one of the
     *                 aforementioned types.
     * @return const CommonDiscriminatorMember instance. 
     */
    RTPS_DllAPI static const CommonDiscriminatorMember build_common_discriminator_member(
            UnionDiscriminatorFlag member_flags,
            const TypeIdentifier& type_id);

    /**
     * @brief Build CompleteDiscriminatorMember instance.
     *
     * @param[in] common CommonDiscriminatorMember.
     * @param[in] ann_builtin Verbatim annotation.
     * @param[in] ann_custom Applied annotations.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given CommonDiscriminatorMember is inconsistent (only in Debug build mode).
     *              2. Any given AppliedAnnotation is inconsistent (only in Debug build mode).
     * @return const CompleteDiscriminatorMember instance.
     */
    RTPS_DllAPI static const CompleteDiscriminatorMember build_complete_discriminator_member(
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
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            not consistent (only in Debug build mode).
     * @return const CompleteUnionHeader instance.
     */
    RTPS_DllAPI static const CompleteUnionHeader build_complete_union_header(
            const CompleteTypeDetail& detail);

    /**
     * MinimalUnionHeader constructed from CompleteUnionHeader.
     */

    /**
     * @brief Build CompleteUnionType instance.
     *
     * @param[in] union_flags 
     * @param[in] header 
     * @param[in] discriminator 
     * @param[in] member_seq 
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given UnionTypeFlags are not consistent (only in Debug build mode).
     *              2. Given CompleteUnionHeader is not consistent (only in Debug build mode).
     *              3. Given CompleteDiscriminatorMember inconsistent (only in Debug build mode).
     *              4. Given CompleteUnionMemberSeq is not consistent (only in Debug build mode).
     * @return const 
     */
    RTPS_DllAPI static const CompleteUnionType build_complete_union_type(
            UnionTypeFlag union_flags,
            const CompleteUnionHeader& header,
            const CompleteDiscriminatorMember& discriminator,
            const CompleteUnionMemberSeq& member_seq);

    /**
     * MinimalUnionType constructed from CompleteUnionType.
     */

    /*************** Annotation: ****************************************/

    /**
     * @brief Build CommonAnnotationParameter instance.
     *
     * @param[in] member_flags AnnotationParameterFlag: empty. No flags apply. It must be zero.
     * @param[in] member_type_id Member TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given AnnotationParameterFlag are not empty.
     *              2. Given TypeIdentifier is not consistent (only in Debug build mode).
     * @return const CommonAnnotationParameter instance.
     */
    RTPS_DllAPI static const CommonAnnotationParameter build_common_annotation_parameter(
            AnnotationParameterFlag member_flags,
            const TypeIdentifier& member_type_id);

    /**
     * @brief Build CompleteAnnotationParameter instance.
     *
     * @param[in] common CommonAnnotationParameter.
     * @param[in] name Member name.
     * @param[in] default_value Annotation default value.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given CommonAnnotationParameter is inconsistent (only in Debug build mode).
     *              2. CommonAnnotationParameter TypeIdentifier is inconsistent with AnnotationParameterValue type (only
     *                 in Debug build mode).
     * @return const CompleteAnnotationParameter instance.
     */
    RTPS_DllAPI static const CompleteAnnotationParameter build_complete_annotation_parameter(
            const CommonAnnotationParameter& common,
            const MemberName& name,
            const AnnotationParameterValue& default_value);

    /**
     * @brief Add CompleteAnnotationParameter to sequence.
     *
     * @param[in out] sequence Sequence to be modified.
     * @param[in] param Complete annotation parameter to be added.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given
     *            CompleteAnnotationParameter is not consistent (only in Debug build mode).
     */
    RTPS_DllAPI static void add_complete_annotation_parameter(
            CompleteAnnotationParameterSeq& sequence,
            const CompleteAnnotationParameter& param);

    /**
     * MinimalAnnotationParameter constructed from CompleteAnnotationParameter.
     */

    /**
     * @brief Build CompleteAnnotationHeader instance.
     *
     * @param[in] annotation_name Qualified annotation type name.
     * @return const CompleteAnnotationHeader instance.
     */
    RTPS_DllAPI static const CompleteAnnotationHeader build_complete_annotation_header(
            const QualifiedTypeName& annotation_name);

    /**
     * MinimalAnnotationHeader constructed from CompleteAnnotationHeader: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteAnnotationType instance.
     *
     * @param[in] annotation_flag Unused. No flags apply. It must be 0.
     * @param[in] header CompleteAnnotationHeader.
     * @param[in] member_seq CompleteAnnotationParameter sequence.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Any annotation flag is set.
     *              2. Any CompleteAnnotationParameter in the sequence is inconsistent (only in Debug build mode).
     * @return const CompleteAnnotationType instance.
     */
    RTPS_DllAPI static const CompleteAnnotationType build_complete_annotation_type(
            AnnotationTypeFlag annotation_flag,
            const CompleteAnnotationHeader& header,
            const CompleteAnnotationParameterSeq& member_seq);

    /**
     * MinimalAnnotationType constructed from CompleteAnnotationType.
     */

    /*************** Alias: *********************************************/

    /**
     * @brief Build CommonAliasBody instance.
     *
     * @param[in] related_flags AliasMemberFlag: unused. No flags apply. It must be 0.
     * @param[in] related_type Related TypeIdentifier.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Any alias member flag is set.
     *              2. Non-consistent TypeIdentifier (only in Debug build mode).
     * @return const CommonAliasBody instance.
     */
    RTPS_DllAPI static const CommonAliasBody build_common_alias_body(
            AliasMemberFlag related_flags,
            const TypeIdentifier& related_type);

    /**
     * @brief Build CompleteAliasBody instance.
     *
     * @param[in] common CommonAliasBody.
     * @param[in] ann_builtin Applied builtin member annotations: unit, max, min, range, hashid
     * @param[in] ann_custom Applied custom annotations
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Given CommonAliasBody is inconsistent (only Debug build mode).
     *              2. AppliedAnnotationSeq is inconsistent (only Debug build mode).
     * @return const CompleteAliasBody instance.
     */
    RTPS_DllAPI static const CompleteAliasBody build_complete_alias_body(
            const CommonAliasBody& common,
            const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /**
     * MinimalAliasBody constructed from CompleteAliasBody.
     */

    /**
     * @brief Build CompleteAliasHeader instance.
     *
     * @param[in] detail Complete type detail.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            inconsistent (only in Debug build mode).
     * @return const CompleteAliasHeader instance.
     */
    RTPS_DllAPI static const CompleteAliasHeader build_complete_alias_header(
            const CompleteTypeDetail& detail);

    /**
     * MinimalAliasHeader constructed from CompleteAliasHeader: empty. Available for future extension.
     */

    /**
     * @brief Build CompleteAliasType instance.
     *
     * @param[in] alias_flags Alias type flags: unused. No flags apply. It must be zero.
     * @param[in] header CompleteAliasHeader.
     * @param[in] body CompleteAliasBody.
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if:
     *              1. Any alias type flag is set.
     *              2. Inconsistent header and/or body (only in Debug build mode).
     * @return const CompleteAliasType instance.
     */
    RTPS_DllAPI static const CompleteAliasType build_complete_alias_type(
            AliasTypeFlag alias_flags,
            const CompleteAliasHeader& header,
            const CompleteAliasBody& body);

    /**
     * MinimalAliasType constructed from CompleteAliasType.
     */

    /*************** Collections: ***************************************/

    /**
     * @brief Build CompleteElementDetail instance.
     *
     * @param[in] ann_builtin Applied builtin member annotations: unit, max, min, range, hashid
     * @param[in] ann_custom Applied custom annotations
     * @exception eprosima::fastdds::dds::xtypesv1_3::InvalidArgumentError exception if AppliedAnnotationSeq is
     *            inconsistent (only Debug build mode).
     * @return const CompleteElementDetail instance.
     */
    RTPS_DllAPI static const CompleteElementDetail build_complete_element_detail(
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
            eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom);

    /*************** Auxiliary public methods ***************************/

    /**
     * @brief Calculate the MD5 hash of the provided name.
     *
     * @param[in] name String which hash is calculated.
     * @return const NameHash Hash of the given string.
     */
    RTPS_DllAPI static const NameHash name_hash(
            const std::string& name);

private:

    // Class with only static methods
    TypeObjectUtils() = delete;
    ~TypeObjectUtils() = delete;

protected:

    /*************** Auxiliary methods ***************************/

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
     * @param[in out] type_flag Bitmask to be set.
     * @param[in] extensibility_kind ExtensibilityKind.
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
     * @param[in] type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is fully-descriptive. false otherwise.
     */
    static bool is_fully_descriptive_type_identifier(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check if a given TypeIdentifier is direct hash.
     *        XTypes v1.3 Clause 7.3.4.6.3
     *        These are HASH TypeIdentifiers with discriminator EK_MINIMAL, EK_COMPLETE or TI_STRONG_COMPONENT.
     *
     * @param[in] type_identifier TypeIdentifier to check.
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
     * @param[in] type_identifier TypeIdentifier to check.
     * @return true if the given TypeIdentifier is indirect hash. false otherwise.
     */
    static bool is_indirect_hash_type_identifier(
            const TypeIdentifier& type_identifier);

    /*************** Consistency methods (Debug) ***************************/

    /**
     * TypeObjectHashId is always consistent. Default constructor already sets the discriminator to one valid value.
     * Union setters prevent changing the discriminator value without setting the corresponding union member.
     */

    /**
     * @brief Check SBound consistency: must be different from 0.
     *
     * @param[in] bound SBound to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given SBound is not
     *            consistent.
     */
    static void s_bound_consistency(
            SBound bound);

    /**
     * @brief Check LBound consistency: must be greater than 255.
     *
     * @param[in] bound LBound to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given LBound is not
     *            consistent.
     */
    static void l_bound_consistency(
            LBound bound);

    /**
     * @brief Check that the array_bound_seq is consistent: non-empty.
     *
     * @tparam T Either SBoundSeq or LBoundSeq
     * @param[in] array Sequence to be checked.
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

    /**
     * @brief Check SBoundSeq consistency.
     *
     * @param[in] bound_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given SBoundSeq is
     *            not consistent.
     */
    static void s_bound_seq_consistency(
            const SBoundSeq& bound_seq);

    /**
     * @brief Check LBoundSeq consistency.
     *
     * @param[in] bound_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given LBoundSeq is
     *            not consistent.
     */
    static void l_bound_seq_consistency(
            const LBoundSeq& bound_seq);

    /**
     * @brief Check MemberFlag consistency: At least one of the bits corresponding to the try construct annotation must
     *        be set.
     * 
     * @param[in] member_flags MemberFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given MemberFlag is not
     *            consistent.
     */
    static void member_flag_consistency(
            MemberFlag member_flags);

    /**
     * @brief Check StructMemberFlag consistency: MemberFlag consistency (try construct annotation).
     *        XTypes v1.3 Clause 7.2.2.4.4.4.8 Key members shall never be optional, and they shall always have their
     *        "must understand" attribute set to true.
     *
     * @param[in] member_flags MemberFlag to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given StructMemberFlag is not
     *            consistent.
     */
    static void struct_member_flag_consistency(
            StructMemberFlag member_flags);

    /**
     * @brief Check TypeFlag consistency: exactly one extensibility flag is set.
     *
     * @param[in] type_flag Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeFlag
     *            is not consistent.
     */
    static void type_flag_consistency(
            TypeFlag type_flag);

    /**
     * @brief Check PlainCollectionHeader consistency:
     *          - CollectionElementFlag consistent
     *          - Consistent EquivalenceKind
     *
     * @param[in] header PlainCollectionHeader to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainCollectionHeader
     *            is not consistent.
     */
    static void plain_collection_header_consistency(
            const PlainCollectionHeader& header);

    /**
     * @brief Check consistency between a given PlainCollectionHeader and the related TypeIdentifier:
     *        1. TypeIdentifier initialized
     *        2. Consistency of EquivalenceKinds
     *
     * @param[in] header PlainCollectionHeader to be checked.
     * @param[in] element_identifier TypeIdentifier to be checked.
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
     *        In Debug build mode, this method also checks that the string/wstring bound is consistent.
     *
     * @param[in] key_identifier TypeIdentifier to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeIdentifier is not
     *            consistent.
     */
    static void map_key_type_identifier_consistency(
            const TypeIdentifier& key_identifier);

    /**
     * @brief Check StringSTypeDefn consistency.
     *
     * @param[in] string Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given StringSTypeDefn is not
     *            consistent.
     */
    static void string_sdefn_consistency(
            const StringSTypeDefn& string);

    /**
     * @brief Check StringLTypeDefn consistency.
     *
     * @param[in] string Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given StringLTypeDefn is not
     *            consistent.
     */
    static void string_ldefn_consistency(
            const StringLTypeDefn& string);

    /**
     * @brief Check PlainSequenceSElemDefn consistency.
     *
     * @param[in] plain_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainSequenceSElemDefn
     *            is not consistent.
     */
    static void seq_sdefn_consistency(
            const PlainSequenceSElemDefn& plain_seq);

    /**
     * @brief Check PlainSequenceLElemDefn consistency.
     *
     * @param[in] plain_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainSequenceLElemDefn
     *            is not consistent.
     */
    static void seq_ldefn_consistency(
            const PlainSequenceLElemDefn& plain_seq);

    /**
     * @brief Check PlainArraySElemDefn consistency.
     *
     * @param[in] plain_array Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainArraySElemDefn is
     *            not consistent.
     */
    static void array_sdefn_consistency(
            const PlainArraySElemDefn& plain_array);

    /**
     * @brief Check PlainArrayLElemDefn consistency.
     *
     * @param[in] plain_array Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainArrayLElemDefn is
     *            not consistent.
     */
    static void array_ldefn_consistency(
            const PlainArrayLElemDefn& plain_array);

    /**
     * @brief Check PlainMapSTypeDefn consistency.
     *
     * @param[in] plain_map Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainMapSTypeDefn is
     *            not consistent.
     */
    static void map_sdefn_consistency(
            const PlainMapSTypeDefn& plain_map);

    /**
     * @brief Check PlainMapLTypeDefn consistency.
     *
     * @param[in] plain_map Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given PlainMapLTypeDefn is
     *            not consistent.
     */
    static void map_ldefn_consistency(
            const PlainMapLTypeDefn& plain_map);

    /**
     * @brief Check TypeIdentifier consistency.
     *
     * @param[in] plain_map Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeIdentifier is
     *            not consistent.
     */
    static void type_identifier_consistency(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check AppliedAnnotation consistency.
     *
     * @param[in] applied_annotation Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given AppliedAnnotation is
     *            not consistent.
     */
    static void applied_annotation_consistency(
            const AppliedAnnotation& applied_annotation);

    /**
     * @brief Check AppliedAnnotationSeq consistency.
     *
     * @param[in] applied_annotation_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given AppliedAnnotationSeq is
     *            not consistent.
     */
    static void applied_annotation_seq_consistency(
            const AppliedAnnotationSeq& applied_annotation_seq);

    /**
     * @brief Check CommonStructMember consistency.
     *
     * @param[in] common_struct_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CommonStructMember is
     *            not consistent.
     */
    static void common_struct_member_consistency(
            const CommonStructMember& common_struct_member);

    /**
     * @brief Check CompleteMemberDetail consistency.
     *
     * @param[in] complete_member_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteMemberDetail is
     *            not consistent.
     */
    static void complete_member_detail_consistency(
            const CompleteMemberDetail& complete_member_detail);

    /**
     * @brief Check CompleteTypeDetail consistency.
     *
     * @param[in] complete_type_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteTypeDetail is
     *            not consistent.
     */
    static void complete_type_detail_consistency(
            const CompleteTypeDetail& complete_type_detail);

    /**
     * @brief Check CompleteStructMember consistency.
     *
     * @param[in] complete_struct_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteStructMember is
     *            not consistent.
     */
    static void complete_struct_member_consistency(
            const CompleteStructMember& complete_struct_member);

    /**
     * @brief Check CompleteStructMemberSeq consistency.
     *
     * @param[in] complete_struct_member_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteStructMemberSeq
     *            is not consistent.
     */
    static void complete_struct_member_seq_consistency(
            const CompleteStructMemberSeq& complete_struct_member_seq);

    /**
     * @brief Check CompleteStructHeader consistency.
     *
     * @param[in] complete_struct_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteStructHeader
     *            is not consistent.
     */
    static void complete_struct_header_consistency(
            const CompleteStructHeader& complete_struct_header);

    /**
     * @brief Check CommonUnionMember consistency.
     *
     * @param[in] common_union_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CommonUnionMember
     *            is not consistent.
     */
    static void common_union_member_consistency(
            const CommonUnionMember& common_union_member);

    /**
     * @brief Check CompleteUnionMember consistency.
     *
     * @param[in] complete_union_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteUnionMember
     *            is not consistent.
     */
    static void complete_union_member_consistency(
            const CompleteUnionMember& complete_union_member);

    /**
     * @brief Check CompleteUnionMemberSeq consistency.
     *
     * @param[in] complete_union_member_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given CompleteUnionMemberSeq
     *            is not consistent.
     */
    static void complete_union_member_seq_consistency(
            const CompleteUnionMemberSeq& complete_union_member_seq);

    /**
     * @brief Check discriminator TypeIdentifier consistency.
     *
     * @param[in] type_id Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeIdentifier
     *            is not consistent.
     */
    static void common_discriminator_member_type_identifier_consistency(
            const TypeIdentifier& type_id);

    /**
     * @brief Check CommonDiscriminatorMember consistency.
     *
     * @param[in] common_discriminator_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CommonDiscriminatorMember is not consistent.
     */
    static void common_discriminator_member_consistency(
            const CommonDiscriminatorMember& common_discriminator_member);

    /**
     * @brief Check CompleteUnionHeader consistency.
     *
     * @param[in] complete_union_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteUnionHeader is not consistent.
     */
    static void complete_union_header_consistency(
            const CompleteUnionHeader& complete_union_header);

    /**
     * @brief Check CompleteDiscriminatorMember consistency.
     *
     * @param[in] complete_discriminator_member Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteDiscriminatorMember is not consistent.
     */
    static void complete_discriminator_member_consistency(
            const CompleteDiscriminatorMember& complete_discriminator_member);

    /**
     * @brief Check empty flags consistency.
     *
     * @tparam T Either MemberFlag or TypeFlag.
     * @param[in] flags Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
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
     * @brief Check that the annotation value is of the same type as the given TypeIdentifier.
     *
     * @param type_id TypeIdentifier.
     * @param value AnnotationParameterValue.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given TypeIdentifier is not
     *            consistent with the given value.
     */
    static void common_annotation_parameter_type_identifier_default_value_consistency(
            const TypeIdentifier& type_id,
            const AnnotationParameterValue& value);

    /**
     * @brief Check CommonAnnotationParameter consistency.
     *
     * @param[in] common_annotation_parameter Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CommonAnnotationParameter is not consistent.
     */
    static void common_annotation_parameter_consistency(
            const CommonAnnotationParameter& common_annotation_parameter);

    /**
     * @brief Check CompleteAnnotationParameter consistency.
     *
     * @param[in] complete_annotation_parameter Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteAnnotationParameter is not consistent.
     */
    static void complete_annotation_parameter_consistency(
            const CompleteAnnotationParameter& complete_annotation_parameter);

    /**
     * @brief Check CompleteAnnotationParameterSeq consistency.
     *
     * @param[in] complete_annotation_parameter_seq Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteAnnotationParameterSeq is not consistent.
     */
    static void complete_annotation_parameter_seq_consistency(
            const CompleteAnnotationParameterSeq& complete_annotation_parameter_seq);

    /**
     * @brief Check CommonAliasBody consistency.
     *
     * @param[in] common_alias_body Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CommonAliasBody is not consistent.
     */
    static void common_alias_body_consistency(
            const CommonAliasBody& common_alias_body);

    /**
     * @brief Check CompleteAliasBody consistency.
     *
     * @param[in] complete_alias_body Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteAliasBody is not consistent.
     */
    static void complete_alias_body_consistency(
            const CompleteAliasBody& complete_alias_body);

    /**
     * @brief Check CompleteAliasHeader consistency.
     *
     * @param[in] complete_alias_header Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteAliasHeader is not consistent.
     */
    static void complete_alias_header_consistency(
            const CompleteAliasHeader& complete_alias_header);

    /**
     * @brief Check CompleteElementDetail consistency.
     *
     * @param[in] complete_element_detail Instance to be checked.
     * @exception eprosima::fastdds::dds::xtypes1_3::InvalidArgumentError exception if the given
     *            CompleteElementDetail is not consistent.
     */
    static void complete_element_detail_consistency(
            const CompleteElementDetail& complete_element_detail);

};

} // xtypes1_3
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_
