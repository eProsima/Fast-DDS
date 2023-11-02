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
 * This file contains the required classes to keep a TypeObject/TypeIdentifier registry.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes1_3::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes1_3::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

template<>
struct hash<eprosima::fastdds::dds::xtypes1_3::TypeIdentfierWithSize>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes1_3::TypeIdentfierWithSize& k) const
    {
        return static_cast<size_t>(k.typeobject_serialized_size());
    }

};

} // std

namespace eprosima {
namespace fastdds {
namespace dds {

namespace xtypes1_3 {

class TypeObjectUtils;

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

// TypeObject information
struct TypeRegistryEntry
{
    // TypeObject
    TypeObject type_object_;
    // TypeObject serialized size
    uint32_t type_object_serialized_size_;

    bool operator !=(
        const TypeRegistryEntry& entry);

};

struct TypeObjectPair
{
    // Minimal TypeObject
    MinimalTypeObject minimal_type_object;
    // Complete TypeObject
    CompleteTypeObject complete_type_object;
};

// Class which holds the TypeObject registry, including every TypeIdentifier (plain and non-plain types), every
// non-plain TypeObject and the non-plain TypeObject serialized sizes.
class TypeObjectRegistry
{

    friend class TypeObjectUtils;

public:

    /**
     * @brief Register a local TypeObject.
     *        The MinimalTypeObject is generated from the CompleteTypeObject, and both are registered into the registry
     *        with the corresponding TypeIdentifiers and TypeObject serialized sizes.
     *
     * @pre type_name must not be empty.
     * @pre complete_type_object must be consistent (only checked in Debug build mode).
     *
     * @param[in] type_name Name of the type being registered.
     * @param[in] complete_type_object CompleteTypeObject related to the given type name.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with the
     *                      given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given type_name is empty or if the type_object
     *                      is inconsistent.
     */
    RTPS_DllAPI ReturnCode_t register_type_object(
            const std::string& type_name,
            const CompleteTypeObject& complete_type_object);

    /**
     * @brief Register an indirect hash TypeIdentifier.
     *
     * @pre TypeIdentifier must not be a direct hash TypeIdentifier.
     * @pre TypeIdentifier must be consistent (only checked in Debug build mode).
     * @pre type_name must not be empty.
     *
     * @param[in] type_name Name of the type being registered.
     * @param[in] type_identifier TypeIdentier related to the given type name.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is inconsistent or a direct hash
     *                      TypeIdentifier or if the given type_name is empty.
     */
    RTPS_DllAPI ReturnCode_t register_type_identifier(
            const std::string& type_name,
            const TypeIdentifier& type_identifier);

    /**
     * @brief Get the TypeObjects related to the given type name.
     *
     * @param[in] type_name Name of the type being queried.
     * @param[out] type_objects Both complete and minimal TypeObjects related with the given type_name.
     * @return ReturnCode_t RETCODE_OK if the TypeObjects are found in the registry.
     *                      RETCODE_NO_DATA if the given type_name has not been registered.
     *                      RETCODE_BAD_PARAMETER if the type_name correspond to a indirect hash TypeIdentifier.
     */
    RTPS_DllAPI ReturnCode_t get_type_objects(
            const std::string& type_name,
            TypeObjectPair& type_objects);

    /**
     * @brief Get the TypeIdentifiers related to the given type name.
     *
     * @param[in] type_name Name of the type being queried.
     * @param[out] type_identifiers For direct hash TypeIdentifiers, both minimal and complete TypeIdentifiers are
     *                              returned.
     *                              For indirect hash TypeIdentifiers, only the corresponding TypeIdentifier is returned
     * @return ReturnCode_t RETCODE_OK if the TypeIdentifiers are found in the registry.
     *                      RETCODE_NO_DATA if the type_name has not been registered.
     */
    RTPS_DllAPI ReturnCode_t get_type_identifiers(
            const std::string& type_name,
            TypeIdentifierPair& type_identifiers);

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
    // Only DomainParticipantFactory is allowed to instantiate the TypeObjectRegistry class.
    // It cannot be protected as the standard library needs to access the constructor to allocate the resources.
    // Rule of zero: resource managing types.
    TypeObjectRegistry() = default;
#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

protected:

    /**
     * @brief Register a remote TypeObject.
     *        This auxiliary method can might register only the minimal TypeObject and TypeIdentifier or register both
     *        TypeObjects constructing the minimal from the complete TypeObject information.
     *
     * @pre TypeIdentifier discriminator must match TypeObject discriminator.
     *      TypeIdentifier consistency is only checked in Debug build mode.
     *
     * @param[in] type_identifier TypeIdentifier to register.
     * @param[in] type_object Related TypeObject being registered.
     * @return ReturnCode_t RETCODE_OK if correctly registered.
     *                      RETCODE_PRECONDITION_NOT_MET if the discriminators differ.
     *                      RETCODE_PRECONDITION_NOT_MET if the TypeIdentifier is not consistent with the given
     *                      TypeObject (only in Debug build mode).
     */
    ReturnCode_t register_type_object(
            const TypeIdentifier& type_identifier,
            const TypeObject& type_object);

    /**
     * @brief Get both the minimal and complete TypeObject related to the given TypeIdentifier.
     *
     * @pre TypeIdentifier must be a direct hash TypeIdentifier.
     *
     * @param[in] type_identifier TypeIdentifier being queried.
     * @param[out] type_objects TypeObjects related with the given TypeIdentifier.
     * @return ReturnCode_t RETCODE_OK if the TypeObject is found within the registry.
     *                      RETCODE_NO_DATA if the given TypeIdentifier is not found in the registry.
     */
    ReturnCode_t get_type_object(
            const TypeIdentifier& type_identifier,
            TypeObjectPair& type_objects);

    /**
     * @brief Get the TypeInformation related to a specific type_name.
     *
     * @param[in] type_name Type which type information is queried.
     * @param[out] type_information Related TypeInformation for the given type name.
     * @return ReturnCode_t RETCODE_OK if the type_name is found within the registry.
     *                      RETCODE_NO_DATA if the given type_name is not found.
     *                      RETCODE_BAD_PARAMETER if the given type name corresponds to a indirect hash TypeIdentifier.
     */
    ReturnCode_t get_type_information(
            const std::string& type_name,
            TypeInformation& type_information);

    /**
     * @brief Check if two given types are compatible according to the given TypeConsistencyEnforcement QoS.
     *
     * @param[in] type_identifiers Pair of TypeIdentifiers to check compatibility.
     * @param[in] type_consistency_qos TypeConsistencyEnforcement QoS to apply.
     * @return ReturnCode_t RETCODE_OK if the two types are compatible.
     *                      RETCODE_ERROR if the types are not compatible according to the TypeConsistencyEnforcement
     *                      QoS.
     */
    ReturnCode_t are_types_compatible(
            const TypeIdentifierPair& type_identifiers,
            const TypeConsistencyEnforcementQosPolicy& type_consistency_qos);

    /**
     * @brief Get the type dependencies of the given type identifiers.
     *
     * @param[in] type_identifiers Sequence with the queried TypeIdentifiers.
     * @param[out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any given TypeIdentifier is unknown to the registry.
     *                      RETCODE_BAD_PARAMETER if any given TypeIdentifier is not a direct hash.
     */
    ReturnCode_t get_type_dependencies(
            const TypeIdentifierSeq& type_identifiers,
            std::unordered_set<TypeIdentfierWithSize> type_dependencies);

    /**
     * @brief Check if the given TypeIdentifier is known by the registry.
     *
     * @param[in] type_identifier TypeIdentifier to query.
     * @return ReturnCode_t RETCODE_OK if TypeIdentifier is known.
     *                      RETCODE_NO_DATA if TypeIdentifier is unknown.
     */
    ReturnCode_t is_type_identifier_known(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Check if a given TypeIdentifier corresponds to a builtin annotation.
     *
     * @param[in] type_identifier TypeIdentifier to check.
     * @return true if the TypeIdentifier is from a builtin annotation. false otherwise.
     */
    bool is_builtin_annotation(
            const TypeIdentifier& type_identifier);

    /**
     * @brief Calculate the TypeIdentifier given a TypeObject.
     *
     * @param[in] type_object TypeObject which is to be hashed.
     * @param[out] type_object_serialized_size
     * @return const TypeIdentifier related with the given TypeObject.
     */
    const TypeIdentifier get_type_identifier(
            const TypeObject& type_object,
            uint32_t& type_object_serialized_size);

    /**
     * @brief Build minimal TypeObject given a CompleteTypeObject.
     *
     * @param[in] complete_type_object CompleteTypeObject.
     * @return const minimal TypeObject instance.
     */
    const TypeObject build_minimal_from_complete_type_object(
            const CompleteTypeObject& complete_type_object);

    /**
     * @brief Build MinimalAliasType given a CompleteAliasType.
     *
     * @param[in] complete_alias_type CompleteAliasType.
     * @return const MinimalAliasType instance.
     */
    const MinimalAliasType build_minimal_from_complete_alias_type(
            const CompleteAliasType& complete_alias_type);

    /**
     * @brief Build MinimalAnnotationType given a CompleteAnnotationType.
     *
     * @param[in] complete_annotation_type CompleteAnnotationType.
     * @return const MinimalAnnotationType instance.
     */
    const MinimalAnnotationType build_minimal_from_complete_annotation_type(
            const CompleteAnnotationType& complete_annotation_type);

    /**
     * @brief Build MinimalStructType given a CompleteStructType.
     *
     * @param[in] complete_struct_type CompleteStructType.
     * @return const MinimalStructType instance.
     */
    const MinimalStructType build_minimal_from_complete_struct_type(
            const CompleteStructType& complete_struct_type);

    /**
     * @brief Build MinimalUnionType given a CompleteUnionType.
     *
     * @param[in] complete_union_type CompleteUnionType.
     * @return const MinimalUnionType instance.
     */
    const MinimalUnionType build_minimal_from_complete_union_type(
            const CompleteUnionType& complete_union_type);

    /**
     * @brief Build MinimalBitsetType given a CompleteBitsetType.
     *
     * @param[in] complete_bitset_type CompleteBitsetType.
     * @return const MinimalBitsetType instance.
     */
    const MinimalBitsetType build_minimal_from_complete_bitset_type(
            const CompleteBitsetType& complete_bitset_type);

    /**
     * @brief Build MinimalSequenceType given a CompleteSequenceType.
     *
     * @param[in] complete_sequence_type CompleteSequenceType.
     * @return const MinimalSequenceType instance.
     */
    const MinimalSequenceType build_minimal_from_complete_sequence_type(
            const CompleteSequenceType& complete_sequence_type);

    /**
     * @brief Build MinimalArrayType given a CompleteArrayType.
     *
     * @param[in] complete_array_type CompleteArrayType.
     * @return const MinimalArrayType instance.
     */
    const MinimalArrayType build_minimal_from_complete_array_type(
            const CompleteArrayType& complete_array_type);

    /**
     * @brief Build MinimalMapType given a CompleteMapType.
     *
     * @param[in] complete_map_type CompleteMapType.
     * @return const MinimalMapType instance.
     */
    const MinimalMapType build_minimal_from_complete_map_type(
            const CompleteMapType& complete_map_type);

    /**
     * @brief Build MinimalEnumeratedType given a CompleteEnumeratedType.
     *
     * @param[in] complete_enumerated_type CompleteEnumeratedType.
     * @return const MinimalEnumeratedType instance.
     */
    const MinimalEnumeratedType build_minimal_from_complete_enumerated_type(
            const CompleteEnumeratedType& complete_enumerated_type);

    /**
     * @brief Build MinimalBitmaskType given a CompleteBitmaskType.
     *
     * @param[in] complete_bitmask_type CompleteBitmaskType.
     * @return const MinimalBitmaskType instance.
     */
    const MinimalBitmaskType build_minimal_from_complete_bitmask_type(
            const CompleteBitmaskType& complete_bitmask_type);

    // Collection of local TypeIdentifiers hashed by type_name.
    // TypeIdentifierPair contains both the minimal and complete TypeObject TypeIdentifiers.
    // In case of indirect hash TypeIdentifiers, type_identifier_2 would be uninitialized (TK_NONE).
    std::unordered_map<std::string, TypeIdentifierPair> local_type_identifiers_;

    // Collection of TypeObjects hashed by its TypeIdentifier.
    // Only direct hash TypeIdentifiers are included in this collection.
    std::unordered_map<TypeIdentifier, TypeRegistryEntry> type_registry_entries_;

};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
