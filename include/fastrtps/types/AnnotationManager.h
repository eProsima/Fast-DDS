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

#ifndef TYPES_ANNOTATION_MANAGER_H
#define TYPES_ANNOTATION_MANAGER_H

#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/MemberId.h>

#include <set>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationManager
{
    std::set<AnnotationDescriptor> annotation_; // Annotations to apply
    using annotation_iterator = std::set<AnnotationDescriptor>::iterator;

    //! auxiliary methods for setters

    template<typename C, typename M>
    void annotation_set(const std::string& id, const C& c, const M& m);

    void annotation_set(const std::string& id, const std::string& new_val);
    void annotation_set(const std::string& id, const char* new_val);

protected:

    //! reset
    void clean()
    {
        annotation_.clear();
    }

    //! annotation state comparisson
    bool operator==(const AnnotationManager& other) const
    {
        return annotation_ == other.annotation_;
    }

    //! retrive a specific annotation by name
    std::pair<annotation_iterator, bool> get_annotation(const std::string& name) const;

public:

    //! retrieve a collection of all annotations
    RTPS_DllAPI const std::set<AnnotationDescriptor>& get_all_annotations() const
    {
        return annotation_;
    }

    // Annotations flags for members

    //! checks if member is bit bound
    RTPS_DllAPI bool annotation_is_bit_bound() const;

    //! checks if member is key
    RTPS_DllAPI bool annotation_is_key() const;

    //! checks if member should be serialized
    RTPS_DllAPI bool annotation_is_non_serialized() const;

    //! checks if member is optional
    RTPS_DllAPI bool annotation_is_optional() const;

    //! checks if client should mandatorily cope with this member
    RTPS_DllAPI bool annotation_is_must_understand() const;

    //! checks if member is value
    RTPS_DllAPI bool annotation_is_value() const;

    //! checks if member is default literal value
    RTPS_DllAPI bool annotation_is_default_literal() const;

    //! checks if member provides a context meaningful position
    RTPS_DllAPI bool annotation_is_position() const;

    //! checks if member is an external reference (pointer)
    RTPS_DllAPI bool annotation_is_external() const;

    // Annotations flags for types

    //! checks if the type is extensible
    RTPS_DllAPI bool annotation_is_extensibility() const;

    //! checks if the type is mutable
    RTPS_DllAPI bool annotation_is_mutable() const;

    //! checks if subclasses can be created
    RTPS_DllAPI bool annotation_is_final() const;

    //! checks if appendability is supported
    RTPS_DllAPI bool annotation_is_appendable() const;

    //! checks if type can be nested
    RTPS_DllAPI bool annotation_is_nested() const;

    //! checks if type is a valid key
    RTPS_DllAPI bool key_annotation() const;

    // Annotations getters

    //! gets annotation value key
    RTPS_DllAPI std::string annotation_get_value() const;

    //! gets default value from annotation
    RTPS_DllAPI std::string annotation_get_default() const;

    //! gets position from annotation
    RTPS_DllAPI uint16_t annotation_get_position() const;

    //! gets bit bound from annotation
    RTPS_DllAPI uint16_t annotation_get_bit_bound() const;

    //! gets extensibility from annotation
    RTPS_DllAPI std::string annotation_get_extensibility() const;

    //! gets type name externally referenced
    RTPS_DllAPI std::string annotation_get_external_typename() const;

    // Annotations setters

    //! sets optional annotation
    RTPS_DllAPI void annotation_set_optional(bool optional);

    //! sets key annotation
    RTPS_DllAPI void annotation_set_key(bool key);

    //! sets 'client must understand' annotation
    RTPS_DllAPI void annotation_set_must_understand(bool must_understand);

    //! sets non-serialized annotation
    RTPS_DllAPI void annotation_set_non_serialized(bool non_serialized);

    //! sets value annotation
    RTPS_DllAPI void annotation_set_value(const std::string& value);

    //! sets default annotation
    RTPS_DllAPI void annotation_set_default(const std::string& default_value);

    //! sets default-literal annotation
    RTPS_DllAPI void annotation_set_default_literal();

    //! sets position annotation
    RTPS_DllAPI void annotation_set_position(uint16_t position);

    //! sets bit-bound annotation
    RTPS_DllAPI void annotation_set_bit_bound(uint16_t bit_bound);

    //! sets extensibility annotation
    RTPS_DllAPI void annotation_set_extensibility(const std::string& extensibility);

    //! sets mutable annotation
    RTPS_DllAPI void annotation_set_mutable();

    //! sets final annotation
    RTPS_DllAPI void annotation_set_final();

    //! sets appendable annotation
    RTPS_DllAPI void annotation_set_appendable();

    //! sets nested annotation
    RTPS_DllAPI void annotation_set_nested(bool nested);

    //! sets external annotation referencing a specific type name
    RTPS_DllAPI void annotation_set_external(const std::string& type_name);

    /**
     * Apply the given annotation to this type (see [standard] section 7.5.2.9.5)
     * @param[in] descriptor @ref AnnotationDescriptor to copy
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI ReturnCode_t apply_annotation(
            const AnnotationDescriptor& descriptor);

    /**
     * Apply the given annotation to this type (see [standard] section 7.5.2.9.5).
     * Creates the new annotation type and initializes the map property single valued
     * @param[in] annotation_name string that provides the underlying annotation type name
     * @param[in] key string for new map entry key
     * @param[in] value string for new map entry value
     * @return standard @ref ReturnCode_t
     * @remarks Convenient constructor that emplaces an @ref AnnotationDescriptor constructed from the arguments
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    /**
     * This operation returns the annotation that corresponds to the specified index,
     * if any (see [standard] section 7.5.2.8.5)
     * @param[out] descriptor @ref AnnotationDescriptor to populate
     * @param[in] idx index associated to the annotation to retrieve
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            std::size_t idx) const;

    /**
     * This operation returns the current number of annotations applied to the type
     * (see [standard] section 7.5.2.8.6)
     * @return number of annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI std::size_t get_annotation_count() const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_ANNOTATION_MANAGER_H
