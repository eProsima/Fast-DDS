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
 * This file contains a class that gives acces to some protected methods of the TypeObjectRegistry.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRYOBSERVER_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRYOBSERVER_HPP_

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectRegistry.hpp>


namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeObjectRegistryObserver {

public:

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
    static ReturnCode_t get_type_object(
            const TypeIdentifier& type_identifier,
            TypeObject& type_object)
    {
        return DomainParticipantFactory::get_instance()->type_object_registry().
            get_type_object(type_identifier, type_object);
    }

    /**
     * @brief Get the type dependencies of the given type identifiers.
     *
     * @param[in] type_identifiers Sequence with the queried TypeIdentifiers.
     * @param[in out] type_dependencies Unordered set of TypeIdentifiers with related TypeObject serialized size.
     * @return ReturnCode_t RETCODE_OK if the operation is successful.
     *                      RETCODE_NO_DATA if any given TypeIdentifier is unknown to the registry.
     *                      RETCODE_BAD_PARAMETER if any given TypeIdentifier is not a direct hash.
     */
    static ReturnCode_t get_type_dependencies(
            const TypeIdentifierSeq& type_identifiers,
            std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
    {
        return DomainParticipantFactory::get_instance()->type_object_registry().
            get_type_dependencies(type_identifiers, type_dependencies);
    }

    /**
     * @brief Check if the given TypeIdentifier is known by the registry.
     *
     * @param[in] type_identifier TypeIdentifier to query.
     * @return true if TypeIdentifier is known. false otherwise.
     */
    static bool is_type_identifier_known(
            const TypeIdentifier& type_identifier)
    {
        return DomainParticipantFactory::get_instance()->type_object_registry().
            is_type_identifier_known(type_identifier);
    }

    /**
     * @brief Check if a given TypeIdentifier corresponds to a builtin annotation.
     *
     * @param[in] type_identifier TypeIdentifier to check.
     * @return true if the TypeIdentifier is from a builtin annotation. false otherwise.
     */
    static bool is_builtin_annotation(
            const TypeIdentifier& type_identifier)
    {
        return DomainParticipantFactory::get_instance()->type_object_registry().
            is_builtin_annotation(type_identifier);
    }

    /**
     * @brief Calculate the TypeIdentifier given a TypeObject.
     *
     * @param[in] type_object TypeObject which is to be hashed.
     * @param[out] type_object_serialized_size
     * @return const TypeIdentifier related with the given TypeObject.
     */
    static const TypeIdentifier calculate_type_identifier(
            const TypeObject& type_object,
            uint32_t& type_object_serialized_size)
    {
        return DomainParticipantFactory::get_instance()->type_object_registry().
            calculate_type_identifier(type_object, type_object_serialized_size);
    }
};



} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRYOBSERVER_HPP_
