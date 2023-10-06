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

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTBUILDER_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTBUILDER_HPP_

#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeObjectBuilder
{
public:

    /**
     * @brief Fill MinimalAnnotationType information.
     *
     * @param [in out] minimal_annotation_type MinimalAnnotationType structure to be filled.
     * @param [in] annotation_flag AnnotationTypeFlag to be set.
     * @param [in] header MinimalAnnotationHeader to be set.
     * @param [in] member_seq MinimalAnnotationParameterSeq to be set.
     */
    RTPS_DllAPI static void minimal_annotation_type_filler(
            MinimalAnnotationType& minimal_annotation_type,
            const AnnotationTypeFlag& annotation_flag,
            const MinimalAnnotationHeader& header,
            const MinimalAnnotationParameterSeq& member_seq);

    /**
     * @brief Create and register both the complete and minimal annotation TypeObject into the TypeObjectRegistry.
     *        This function also registers the associated TypeIdentifiers and the TypeObject serialized sizes.
     *
     * @param [in] annotation_name Name to register in TypeObjectRegistry
     * @param [in] minimal_annotation_type MinimalAnnotationType to set in MinimalTypeObject
     * @param complete_annotation_type CompleteAnnotationType to set in CompleteTypeObject
     */
    RTPS_DllAPI static void create_register_annotation_type_object(
            const std::string& annotation_name,
            const MinimalAnnotationType& minimal_annotation_type,
            const CompleteAnnotationType& complete_annotation_type);

private:

    // Class with only static methods
    TypeObjectBuilder() = delete;
    ~TypeObjectBuilder() = delete;
};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTBUILDER_HPP_
