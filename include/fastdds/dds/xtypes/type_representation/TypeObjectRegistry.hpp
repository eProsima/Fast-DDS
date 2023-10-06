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

#include <map>
#include <memory>
#include <string>

#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

// TypeObject information
struct TypeObjectInfo
{
    // Plain/Minimal TypeIdentifier
    std::shared_ptr<TypeIdentifier> type_identifier_;
    // MinimalTypeObject if the type is non-plain
    std::shared_ptr<MinimalTypeObject> minimal_type_object_;
    // MinimalTypeObject serialized size if the type is non-plain
    uint32_t minimal_type_object_serialized_size_ = 0;
    // CompleteTypeIdentifier if the type is non-plain
    std::shared_ptr<TypeIdentifier> complete_type_identifier_;
    // CompleteTypeObject if the type is non-plain
    std::shared_ptr<CompleteTypeObject> complete_type_object_;
    // CompleteTypeObject serialized size if the type is non-plain
    uint32_t complete_type_object_serialized_size_ = 0;
};

// Class which holds the TypeObject registry, including every TypeIdentifier (plain and non-plain types), every
// non-plain TypeObject and the non-plain TypeObject serialized sizes.
class TypeObjectRegistry
{
public:

    /**
     * @brief Register TypeObject information
     *
     * @param type_name Name of the type which information is registered.
     * @param type_object_info TypeObject information related to the given type name.
     */
    RTPS_DllAPI void register_type_object_info(
            const std::string& type_name,
            const TypeObjectInfo& type_object_info);

    /**
     * @brief Get the type identifiers.
     *
     * @param type_name Type queried.
     * @return TypeIdentifierPair If the queried type is plain, only type_identifier1 would provide meaningful
     *                               information, being type_identifier2 invalid (TK_NONE).
     *                            If the queried type is non-plain, type_identifier1 holds the TypeIdenfier
     *                               corresponding to the MinimalTypeObject and typeidentifier2 the TypeIdentifier of
     *                               the CompleteTypeObject.
     */
    RTPS_DllAPI TypeObjectInfo get_type_object_info(
            const std::string& type_name);

protected:

    // Only DomainParticipantFactory is allowed to instantiate the TypeObjectRegistry class
    TypeObjectRegistry() = default;
    ~TypeObjectRegistry();

    // Collection of TypeObjectInfo accessed by type name
    std::map<std::string, TypeObjectInfo> type_object_registry_;

};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
