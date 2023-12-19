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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_HPP

#include <memory>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace xtypes {
class TypeObject;
} // namespace xtypes

class RTPS_DllAPI DynamicTypeBuilderFactory : public std::enable_shared_from_this<DynamicTypeBuilderFactory>
{
public:

    /*!
     * Returns the singleton factory object
     * @remark This method is non thread-safe.
     * @return @ref DynamicTypeBuilderFactory reference.
     */
    static traits<DynamicTypeBuilderFactory>::ref_type get_instance();

    /*!
     * Resets the singleton reference.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK is always returned.
     * @retval RETCODE_BAD_PARAMETER if singleton reference is currently nil.
     */
    static ReturnCode_t delete_instance();

    /*!
     * Retrieves the cached @ref DynamicType reference associated to a given primitive
     * @param[in] kind Type identifying the primitive type to retrieve.
     * @return @ref DynamicType reference. Nil reference returned in error case.
     */
    virtual traits<DynamicType>::ref_type get_primitive_type(
            TypeKind kind) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference based on the given @ref TypeDescriptor state.
     * @param[in] descriptor @ref TypeDescriptor to be copied.
     * @return New @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_type(
            traits<TypeDescriptor>::ref_type descriptor) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference based on the given @ref DynamicType reference.
     * @param[in] type @ref DynamicType reference to be used.
     * @return New @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_type_copy(
            traits<DynamicType>::ref_type type) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference based on the given @ref TypeObject instance.
     * @remark Not implemented yet.
     * @param[in] type_object @ref TypeObject instance to be used.
     * @return New @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_type_w_type_object(
            const xtypes::TypeObject& type_object) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing a bounded string type.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * If the value is equal to LENGTH_UNLIMITED, the string type shall be considered to be unbounded.
     * @return new @ref DynamicTypeBuilder reference.. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_string_type(
            uint32_t bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing a bounded wstring type.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * If the value is equal to LENGTH_UNLIMITED, the wstring type shall be considered to be unbounded.
     * @return new @ref DynamicTypeBuilder reference.. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_wstring_type(
            uint32_t bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing a sequence.
     * @param[in] element_type @ref DynamicType reference which becomes the element type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * If the value is equal to LENGTH_UNLIMITED, the sequence type shall be considered to be unbounded.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_sequence_type(
            traits<DynamicType>::ref_type element_type,
            uint32_t bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing an array.
     * @param[in] element_type @ref DynamicType reference which becomes the element type
     * @param[in] bounds `uint32_t` representing the desired dimensions.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_array_type(
            traits<DynamicType>::ref_type element_type,
            const BoundSeq& bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing a map.
     * @param[in] key_type @ref DynamicType reference which becomes the map's key type
     * @param[in] value_type @ref DynamicType reference which becomes the map's value type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * If the value is equal to LENGTH_UNLIMITED, the map type shall be considered to be unbounded.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_map_type(
            traits<DynamicType>::ref_type key_element_type,
            traits<DynamicType>::ref_type element_type,
            uint32_t bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference representing a bitmask
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_bitmask_type(
            uint32_t bound) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference by parsing the type description at the given URL.
     * @remark Not implemented yet.
     * @param[in] URL pointing to the XML description.
     * @param[in] type_name Fully qualified name of the type to be loaded from the document.
     * @param[in] include_paths A collection of URLs to directories to be searched for additional type description
     * documents.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_type_w_uri(
            const std::string& document_url,
            const std::string& type_name,
            const IncludePathSeq& include_paths) = 0;

    /*!
     * Creates a new @ref DynamicTypeBuilder reference by parsing the type description contained in the given string.
     * @remark Not implemented yet.
     * @param[in] String containing the type description.
     * @param[in] type_name Fully qualified name of the type to be loaded from the string.
     * @param[in] include_paths A collection of URLs to directories to be searched for additional type description
     * documents.
     * @return new @ref DynamicTypeBuilder reference. Nil reference returned in error case.
     */
    virtual traits<DynamicTypeBuilder>::ref_type create_type_w_document(
            const std::string& document,
            const std::string& type_name,
            const IncludePathSeq& include_paths) = 0;

    /*!
     * Resets the internal reference if it is cached.
     * @param[in] type @ref DynamicType reference whose internal cached reference to reset.
     * @return standard ReturnCode_t
     * @retval RETCODE_OK is always returned.
     */
    virtual ReturnCode_t delete_type(
            traits<DynamicType>::ref_type type) = 0;

protected:

    DynamicTypeBuilderFactory() = default;

    virtual ~DynamicTypeBuilderFactory() = default;

    traits<DynamicTypeBuilderFactory>::ref_type _this ();

private:

    DynamicTypeBuilderFactory(
            const DynamicTypeBuilderFactory&) = delete;

    DynamicTypeBuilderFactory(
            DynamicTypeBuilderFactory&&) = delete;

    DynamicTypeBuilderFactory& operator =(
            const DynamicTypeBuilderFactory&) = delete;

    DynamicTypeBuilderFactory& operator =(
            DynamicTypeBuilderFactory&&) = delete;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_HPP
