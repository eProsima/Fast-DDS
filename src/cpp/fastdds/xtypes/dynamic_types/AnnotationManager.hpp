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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONMANAGER_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONMANAGER_HPP

#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/Types.hpp>
#include "AnnotationDescriptorImpl.hpp"

#include <set>
#include <string>

namespace eprosima {
namespace fastdds {
namespace dds {

class AnnotationManager
{
    std::set<AnnotationDescriptorImpl> annotation_; // Annotations to apply
    using annotation_iterator = std::set<AnnotationDescriptorImpl>::iterator;

    //! auxiliary methods for setters

    template<typename C, typename M>
    void annotation_set(
            const std::string& id,
            const C& c,
            const M& m);

    void annotation_set(
            const std::string& id,
            const std::string& new_val);
    void annotation_set(
            const std::string& id,
            const char* new_val);

protected:

    //! reset
    void clean()
    {
        annotation_.clear();
    }

    //! annotation state comparison
    bool operator ==(
            const AnnotationManager& other) const
    {
        return true; //TODO(richiware) annotation_ == other.annotation_;
    }

public:

    AnnotationManager() = default;

    AnnotationManager(
            const AnnotationManager& m)
        : annotation_(m.annotation_)
    {
    }

    AnnotationManager(
            AnnotationManager&& m)
        : annotation_(std::move(m.annotation_))
    {
    }

    AnnotationManager& operator =(
            const AnnotationManager& m)
    {
        annotation_ = m.annotation_;
        return *this;
    }

    AnnotationManager& operator =(
            AnnotationManager&& m)
    {
        annotation_ = std::move(m.annotation_);
        return *this;
    }

    //! retrieve a collection of all annotations
    const std::set<AnnotationDescriptorImpl>& get_all_annotations() const
    {
        return annotation_;
    }

    // Annotations flags for members

    //! checks if member is bit bound
    bool annotation_is_bit_bound() const;

    //! checks if member is key
    bool annotation_is_key() const;

    //! checks if member should be serialized
    bool annotation_is_non_serialized() const;

    //! checks if member is optional
    bool annotation_is_optional() const;

    //! checks if client should mandatorily cope with this member
    bool annotation_is_must_understand() const;

    //! checks if member is value
    bool annotation_is_value() const;

    //! checks if member is default literal value
    bool annotation_is_default_literal() const;

    //! checks if member provides a context meaningful position
    bool annotation_is_position() const;

    //! checks if member is an external reference (pointer)
    bool annotation_is_external() const;

    // Annotations flags for types

    //! checks if the type is extensible
    bool annotation_is_extensibility() const;

    //! checks if the type is mutable
    bool annotation_is_mutable() const;

    //! checks if subclasses can be created
    bool annotation_is_final() const;

    //! checks if appendability is supported
    bool annotation_is_appendable() const;

    //! checks if type can be nested
    bool annotation_is_nested() const;

    //! checks if type is a valid key
    bool key_annotation() const;

    // Annotations getters

    //! gets annotation value key
    ObjectName annotation_get_value() const;

    //! gets default value from annotation
    ObjectName annotation_get_default() const;

    //! gets position from annotation
    uint16_t annotation_get_position() const;

    //! gets bit bound from annotation
    uint16_t annotation_get_bit_bound() const;

    //! gets extensibility from annotation
    ObjectName annotation_get_extensibility() const;

    //! gets type name externally referenced
    ObjectName annotation_get_external_typename() const;

    // Annotations setters

    //! sets optional annotation
    void annotation_set_optional(
            bool optional);

    //! sets key annotation
    void annotation_set_key(
            bool key);

    //! sets 'client must understand' annotation
    void annotation_set_must_understand(
            bool must_understand);

    //! sets non-serialized annotation
    void annotation_set_non_serialized(
            bool non_serialized);

    //! sets value annotation
    void annotation_set_value(
            const ObjectName& value);

    //! sets default annotation
    void annotation_set_default(
            const ObjectName& default_value);

    //! sets default-literal annotation
    void annotation_set_default_literal();

    //! sets position annotation
    void annotation_set_position(
            uint16_t position);

    //! sets bit-bound annotation
    void annotation_set_bit_bound(
            uint16_t bit_bound);

    //! sets extensibility annotation
    void annotation_set_extensibility(
            const ObjectName& extensibility);

    //! sets mutable annotation
    void annotation_set_mutable();

    //! sets final annotation
    void annotation_set_final();

    //! sets appendable annotation
    void annotation_set_appendable();

    //! sets nested annotation
    void annotation_set_nested(
            bool nested);

    //! sets external annotation referencing a specific type name
    void annotation_set_external(
            const ObjectName& type_name);

    /**
     * Apply the given annotation to this type (see [standard] section 7.5.2.9.5)
     * @param[in] descriptor @ref AnnotationDescriptorImpl to copy
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t apply_annotation(
            const AnnotationDescriptorImpl& descriptor);

    /**
     * Apply the given annotation to this type (see [standard] section 7.5.2.9.5).
     * Creates the new annotation type and initializes the map property single valued
     * @param[in] annotation_name string that provides the underlying annotation type name
     * @param[in] key string for new map entry key
     * @param[in] value string for new map entry value
     * @return standard @ref ReturnCode_t
     * @remarks Convenient constructor that emplaces an @ref AnnotationDescriptorImpl constructed from the arguments
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const ObjectName& key,
            const ObjectName& value);

    //! retrieve a specific annotation by name
    std::pair<annotation_iterator, bool> get_annotation(
            const std::string& name) const;

    //! retrieve a specific annotation by index
    std::pair<annotation_iterator, bool> get_annotation(
            std::size_t idx) const;

    /**
     * This operation returns the annotation that corresponds to the specified index,
     * if any (see [standard] section 7.5.2.8.5)
     * @param[out] descriptor @ref AnnotationDescriptor to populate
     * @param[in] idx index associated to the annotation to retrieve
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t get_annotation(
            AnnotationDescriptor& annotation,
            uint32_t index) const noexcept;

    /**
     * This operation returns the current number of annotations applied to the type
     * (see [standard] section 7.5.2.8.6)
     * @return number of annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::size_t get_annotation_count() const;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONMANAGER_HPP
