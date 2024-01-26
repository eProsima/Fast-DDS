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

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include <fastrtps/fastrtps_dll.h>

#include <fastdds/xtypes/type_representation/TypeIdentifierWithSizeHashSpecialization.h>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

} // std

namespace eprosima {
namespace fastdds {
namespace dds {

namespace xtypes {

using ReturnCode_t = eprosima::fastdds::dds::ReturnCode_t;

// TypeObject information
struct TypeRegistryEntry
{
    // TypeObject
    TypeObject type_object_;
    // TypeObject serialized size
    uint32_t type_object_serialized_size_ {0};

    bool operator !=(
            const TypeRegistryEntry& entry);

};

// Class which holds the TypeObject registry, including every TypeIdentifier (plain and non-plain types), every
// non-plain TypeObject and the non-plain TypeObject serialized sizes.
class TypeObjectRegistry : public ITypeObjectRegistry
{

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
    ReturnCode_t register_type_object(
            const std::string& type_name,
            const CompleteTypeObject& complete_type_object) override;

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
    ReturnCode_t register_type_identifier(
            const std::string& type_name,
            const TypeIdentifier& type_identifier) override;

    /**
     * @brief Get the TypeObjects related to the given type name.
     *
     * @pre type_name must not be empty.
     *
     * @param[in] type_name Name of the type being queried.
     * @param[out] type_objects Both complete and minimal TypeObjects related with the given type_name.
     * @return ReturnCode_t RETCODE_OK if the TypeObjects are found in the registry.
     *                      RETCODE_NO_DATA if the given type_name has not been registered.
     *                      RETCODE_BAD_PARAMETER if the type_name correspond to a indirect hash TypeIdentifier.
     *                      RETCODE_PRECONDITION_NOT_MET if the type_name is empty.
     */
    ReturnCode_t get_type_objects(
            const std::string& type_name,
            TypeObjectPair& type_objects) override;

    /**
     * @brief Get the TypeIdentifiers related to the given type name.
     *
     * @pre type_name must not be empty.
     *
     * @param[in] type_name Name of the type being queried.
     * @param[out] type_identifiers For direct hash TypeIdentifiers, both minimal and complete TypeIdentifiers are
     *                              returned.
     *                              For indirect hash TypeIdentifiers, only the corresponding TypeIdentifier is returned
     * @return ReturnCode_t RETCODE_OK if the TypeIdentifiers are found in the registry.
     *                      RETCODE_NO_DATA if the type_name has not been registered.
     *                      RETCODE_PRECONDITION_NOT_MET if the type_name is empty.
     */
    ReturnCode_t get_type_identifiers(
            const std::string& type_name,
            TypeIdentifierPair& type_identifiers) override;

    /**
     * @brief Get the TypeObject related to the given TypeIdentifier.
     *
     * @pre TypeIdentifier must be a direct hash TypeIdentifier.
     *
     * @param[in] type_identifier TypeIdentifier being queried.
     * @param[out] type_object TypeObject related with the given TypeIdentifier.
     * @return ReturnCode_t RETCODE_OK if the TypeObject is found within the registry.
     *                      RETCODE_NO_DATA if the given TypeIdentifier is not found in the registry.
     *                      RETCODE_PRECONDITION_NOT_MET if the TypeIdentifier is not a direct hash.
     */
    ReturnCode_t get_type_object(
            const TypeIdentifier& type_identifier,
            TypeObject& type_object) override;

    /**
     * @brief Get the TypeInformation related to a specific type_name.
     *
     * @pre type_name must not be empty.
     *
     * @param[in] type_name Type which type information is queried.
     * @param[out] type_information Related TypeInformation for the given type name.
     * @return ReturnCode_t RETCODE_OK if the type_name is found within the registry.
     *                      RETCODE_NO_DATA if the given type_name is not found.
     *                      RETCODE_BAD_PARAMETER if the given type name corresponds to a indirect hash TypeIdentifier.
     *                      RETCODE_PRECONDITION_NOT_MET if the type_name is empty.
     */
    ReturnCode_t get_type_information(
            const std::string& type_name,
            TypeInformation& type_information);

    /**
     * @brief Get the type dependencies of the given type identifiers.
     *
     * @param[in] type_identifiers Sequence with the queried TypeIdentifiers.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any given TypeIdentifier is unknown to the registry.
     *                      RETCODE_BAD_PARAMETER if any given TypeIdentifier is not a direct hash.
     */
    ReturnCode_t get_type_dependencies(
            const TypeIdentifierSeq& type_identifiers,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies);

    /**
     * @brief Check if the given TypeIdentfierWithSize is known by the registry.
     *
     * @param[in] type_identifier_with_size TypeIdentfierWithSize to query.
     * @return true if TypeIdentfierWithSize is known. false otherwise.
     */
    bool is_type_identifier_known(
            const TypeIdentfierWithSize& type_identifier_with_size);

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
    const TypeIdentifier calculate_type_identifier(
            const TypeObject& type_object,
            uint32_t& type_object_serialized_size);

    /**
     * @brief Register a remote TypeObject.
     *        This auxiliary method might register only the minimal TypeObject and TypeIdentifier or register both
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
     *                      TypeObject.
     */
    ReturnCode_t register_type_object(
            const TypeIdentifier& type_identifier,
            const TypeObject& type_object);

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
     * @brief Get the type dependencies of the given TypeObject.
     *
     * @param[in] type_object TypeObject queried for its dependencies.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     *                      RETCODE_BAD_PARAMETER if any given TypeIdentifier is not a direct hash.
     */
    ReturnCode_t get_dependencies_from_type_object(
            const TypeObject& type_object,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies);

    // Only DomainParticipantFactory is allowed to instantiate the TypeObjectRegistry class.
    // It cannot be protected as the standard library needs to access the constructor to allocate the resources.
    // Rule of zero: resource managing types.
    TypeObjectRegistry();

protected:

    /**
     * @brief Add type dependency to the sequence.
     *
     * @param[in] type_id TypeIdentifier to be added.
     * @param[in out] type_dependencies TypeIdentfierWithSize sequence.
     */
    void add_dependency(
            const TypeIdentifier& type_id,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies);

    /**
     * @brief Get the alias type dependencies.
     *
     * @tparam T Either a CompleteAliasType or MinimalAliasType.
     * @param[in] alias_type Alias Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_alias_dependencies(
            const T& alias_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifier type_id = alias_type.body().common().related_type();
        if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
            add_dependency(type_id, type_dependencies);
            TypeIdentifierSeq type_ids;
            type_ids.push_back(type_id);
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Get the annotation type dependencies.
     *
     * @tparam T Either a CompleteAnnotationType or MinimalAnnotationType.
     * @param[in] annotation_type Annotation Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_annotation_dependencies(
            const T& annotation_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifierSeq type_ids;
        for (auto member : annotation_type.member_seq())
        {
            TypeIdentifier type_id = member.common().member_type_id();
            if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
            {
                add_dependency(type_id, type_dependencies);
                type_ids.push_back(type_id);
            }
        }
        if (!type_ids.empty())
        {
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Get the structure type dependencies.
     *
     * @tparam T Either a CompleteStructType or MinimalStructType.
     * @param[in] struct_type Structure Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_structure_dependencies(
            const T& struct_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifierSeq type_ids;
        TypeIdentifier parent_type_id = struct_type.header().base_type();
        if (TypeObjectUtils::is_direct_hash_type_identifier(parent_type_id))
        {
            add_dependency(parent_type_id, type_dependencies);
            type_ids.push_back(parent_type_id);
        }
        for (auto member : struct_type.member_seq())
        {
            TypeIdentifier type_id = member.common().member_type_id();
            if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
            {
                add_dependency(type_id, type_dependencies);
                type_ids.push_back(type_id);
            }
        }
        if (!type_ids.empty())
        {
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Get the union type dependencies.
     *
     * @tparam T Either a CompleteUnionType or MinimalUnionType.
     * @param[in] union_type Union Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_union_dependencies(
            const T& union_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifierSeq type_ids;
        TypeIdentifier discriminator_type_id = union_type.discriminator().common().type_id();
        if (TypeObjectUtils::is_direct_hash_type_identifier(discriminator_type_id))
        {
            add_dependency(discriminator_type_id, type_dependencies);
            type_ids.push_back(discriminator_type_id);
        }
        for (auto member : union_type.member_seq())
        {
            TypeIdentifier type_id = member.common().type_id();
            if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
            {
                add_dependency(type_id, type_dependencies);
                type_ids.push_back(type_id);
            }
        }
        if (!type_ids.empty())
        {
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Get the sequence/array type dependencies.
     *
     * @tparam T Either a CompleteSequenceType/MinimalSequenceType/CompleteArrayType/MinimalArrayType.
     * @param[in] collection_type Sequence or Array Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_sequence_array_dependencies(
            const T& collection_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifier type_id = collection_type.element().common().type();
        if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
            add_dependency(type_id, type_dependencies);
            TypeIdentifierSeq type_ids;
            type_ids.push_back(type_id);
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Get the map type dependencies.
     *
     * @tparam T Either a CompleteMapType or MinimalmapType.
     * @param[in] map_type Map Type.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any dependent TypeIdentifier is unknown to the registry.
     */
    template<typename T>
    ReturnCode_t get_map_dependencies(
            const T& map_type,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        TypeIdentifierSeq type_ids;
        TypeIdentifier key_type_id = map_type.key().common().type();
        if (TypeObjectUtils::is_direct_hash_type_identifier(key_type_id))
        {
            add_dependency(key_type_id, type_dependencies);
            type_ids.push_back(key_type_id);
        }
        TypeIdentifier element_type_id = map_type.element().common().type();
        if (TypeObjectUtils::is_direct_hash_type_identifier(element_type_id))
        {
            add_dependency(element_type_id, type_dependencies);
            type_ids.push_back(element_type_id);
        }
        if (!type_ids.empty())
        {
            return get_type_dependencies(type_ids, type_dependencies);
        }
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Check if a given name corresponds to a builtin annotation.
     *
     * @param[in] name to check.
     * @return true if the name is from a builtin annotation. false otherwise.
     */
    bool is_builtin_annotation_name(
            const std::string& name);

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

    /**
     * @brief Register TypeIdentifiers corresponding to the primitive types.
     */
    void register_primitive_type_identifiers();

    /**
     * @brief Get Minimal TypeIdentifier from Complete TypeIdentifier.
     *
     * @param complete_type_id Direct hash complete TypeIdentifier
     * @return TypeIdentifier Minimal TypeIdentifier related to the given Complete TypeIdentifier.
     *         Same TypeIdentifier if the given TypeIdentifier is not a direct hash complete TypeIdentifier.
     */
    const TypeIdentifier minimal_from_complete_type_identifier(
            const TypeIdentifier& complete_type_id);

    // Collection of local TypeIdentifiers hashed by type_name.
    // TypeIdentifierPair contains both the minimal and complete TypeObject TypeIdentifiers.
    // In case of indirect hash TypeIdentifiers, type_identifier_2 would be uninitialized (TK_NONE).
    std::unordered_map<std::string, TypeIdentifierPair> local_type_identifiers_;

    // Collection of TypeObjects hashed by its TypeIdentifier.
    // Only direct hash TypeIdentifiers are included in this collection.
    std::unordered_map<TypeIdentifier, TypeRegistryEntry> type_registry_entries_;

    // Mutex to protect concurrent access to collections contained in this class
    std::mutex type_object_registry_mutex_;

};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
