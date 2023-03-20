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

#include <set>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace types {

struct AnnotationManager
{
    std::set<AnnotationDescriptor> annotation_; // Annotations to apply
    using annotation_iterator = std::set<AnnotationDescriptor>::iterator;

    RTPS_DllAPI std::pair<annotation_iterator, bool> get_annotation(const std::string& name) const;

    RTPS_DllAPI const std::set<AnnotationDescriptor>& get_all_annotations() const
    {
        return annotation_;
    }

    // Generic annnotations flags
    RTPS_DllAPI bool annotation_is_bit_bound() const;

    RTPS_DllAPI bool annotation_is_key() const;

    RTPS_DllAPI bool annotation_is_non_serialized() const;

    // Annotations flags for members
    RTPS_DllAPI bool annotation_is_optional() const;

    RTPS_DllAPI bool annotation_is_must_understand() const;

    RTPS_DllAPI bool annotation_is_value() const;

    RTPS_DllAPI bool annotation_is_default_literal() const;

    RTPS_DllAPI bool annotation_is_position() const;

    RTPS_DllAPI bool annotation_is_external() const;

    // Annotations flags for types
    RTPS_DllAPI bool annotation_is_extensibility() const;

    RTPS_DllAPI bool annotation_is_mutable() const;

    RTPS_DllAPI bool annotation_is_final() const;

    RTPS_DllAPI bool annotation_is_appendable() const;

    RTPS_DllAPI bool annotation_is_nested() const;

    RTPS_DllAPI bool key_annotation() const;

    // Annotations getters
    RTPS_DllAPI std::string annotation_get_value() const;

    RTPS_DllAPI std::string annotation_get_default() const;

    RTPS_DllAPI uint16_t annotation_get_position() const;

    RTPS_DllAPI uint16_t annotation_get_bit_bound() const;

    RTPS_DllAPI std::string annotation_get_extensibility() const;

    RTPS_DllAPI std::string annotation_get_external_typename() const;

    // Annotations setters

    //! auxiliary method for all bellow
    template<typename C, typename M>
    void annotation_set(const std::string& id, const C& c, const M& m);

    //! auxiliary method for all bellow
    void annotation_set(const std::string& id, const std::string& new_val);
    void annotation_set(const std::string& id, const char* new_val);

    RTPS_DllAPI void annotation_set_optional(bool optional);

    RTPS_DllAPI void annotation_set_key(bool key);

    RTPS_DllAPI void annotation_set_must_understand(bool must_understand);

    RTPS_DllAPI void annotation_set_non_serialized(bool non_serialized);

    RTPS_DllAPI void annotation_set_value(const std::string& value);

    RTPS_DllAPI void annotation_set_default(const std::string& default_value);

    RTPS_DllAPI void annotation_set_default_literal();

    RTPS_DllAPI void annotation_set_position(uint16_t position);

    RTPS_DllAPI void annotation_set_bit_bound(uint16_t bit_bound);

    RTPS_DllAPI void annotation_set_extensibility(const std::string& extensibility);

    RTPS_DllAPI void annotation_set_mutable();

    RTPS_DllAPI void annotation_set_final();

    RTPS_DllAPI void annotation_set_appendable();

    RTPS_DllAPI void annotation_set_nested(bool nested);

    RTPS_DllAPI void annotation_set_external(const std::string& type_name);

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            MemberId idx) const;

    // TODO: doxygen
    RTPS_DllAPI MemberId get_annotation_count() const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_ANNOTATION_MANAGER_H
