// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_TYPE_DESCRIPTOR_H
#define TYPES_TYPE_DESCRIPTOR_H

#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypesBase.h>

class MemberDescriptor;
class DynamicType;

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeDescriptor
{
protected:

    TypeKind kind_;                         // Type Kind.
    std::string name_;                      // Type Name.
    DynamicType_ptr base_type_;             // SuperType of an structure or base type of an alias type.
    DynamicType_ptr discriminator_type_;    // Discrimination type for a union.
    std::vector<uint32_t> bound_;           // Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr element_type_;          // Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr key_element_type_;      // Key Type for maps.
    std::vector<AnnotationDescriptor*> annotation_; // Annotations to apply

    void clean();

    bool is_type_name_consistent(
            const std::string& sName) const;

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;
    friend class DynamicType;
    friend class MemberDescriptor;
    friend class DynamicDataHelper;

public:

    TypeDescriptor();

    RTPS_DllAPI TypeDescriptor(
            const TypeDescriptor* other);

    RTPS_DllAPI TypeDescriptor(
            const std::string& name,
            TypeKind kind);

    RTPS_DllAPI ~TypeDescriptor();

    ReturnCode_t copy_from(
            const TypeDescriptor* descriptor);

    bool equals(
            const TypeDescriptor* descriptor) const;

    bool is_consistent() const;

    DynamicType_ptr get_base_type() const;

    uint32_t get_bounds(
            uint32_t index = 0) const;

    uint32_t get_bounds_size() const;

    DynamicType_ptr get_discriminator_type() const;

    DynamicType_ptr get_element_type() const;

    DynamicType_ptr get_key_element_type() const;

    TypeKind get_kind() const;

    std::string get_name() const;

    uint32_t get_total_bounds() const;

    void set_kind(
            TypeKind kind);

    void set_name(
            std::string name);

    ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    AnnotationDescriptor* get_annotation(
            const std::string& name) const;

    // Annotations application
    bool annotation_is_extensibility() const;

    bool annotation_is_mutable() const;

    bool annotation_is_final() const;

    bool annotation_is_appendable() const;

    bool annotation_is_nested() const;

    bool annotation_is_bit_bound() const;

    bool annotation_is_key() const;

    bool annotation_is_non_serialized() const;

    // Annotation getters
    std::string annotation_get_extensibility() const;

    bool annotation_get_nested() const;

    uint16_t annotation_get_bit_bound() const;

    bool annotation_get_key() const;

    // Annotation setters
    void annotation_set_extensibility(
            const std::string& extensibility);

    void annotation_set_mutable();

    void annotation_set_final();

    void annotation_set_appendable();

    void annotation_set_nested(
            bool nested);

    void annotation_set_bit_bound(
            uint16_t bit_bound);

    void annotation_set_key(
            bool key);

    void annotation_set_non_serialized(
            bool non_serialized);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
