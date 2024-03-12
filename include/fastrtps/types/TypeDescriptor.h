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

    FASTDDS_EXPORTED_API void clean();

    static bool is_type_name_consistent(
            const std::string& sName);

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;
    friend class DynamicType;
    friend class MemberDescriptor;
    friend class DynamicDataHelper;

public:

    FASTDDS_EXPORTED_API TypeDescriptor();

    FASTDDS_EXPORTED_API TypeDescriptor(
            const TypeDescriptor* other);

    FASTDDS_EXPORTED_API TypeDescriptor(
            const std::string& name,
            TypeKind kind);

    FASTDDS_EXPORTED_API ~TypeDescriptor();

    FASTDDS_EXPORTED_API ReturnCode_t copy_from(
            const TypeDescriptor* descriptor);

    FASTDDS_EXPORTED_API bool equals(
            const TypeDescriptor* descriptor) const;

    FASTDDS_EXPORTED_API bool is_consistent() const;

    FASTDDS_EXPORTED_API DynamicType_ptr get_base_type() const;

    FASTDDS_EXPORTED_API uint32_t get_bounds(
            uint32_t index = 0) const;

    FASTDDS_EXPORTED_API uint32_t get_bounds_size() const;

    FASTDDS_EXPORTED_API DynamicType_ptr get_discriminator_type() const;

    FASTDDS_EXPORTED_API DynamicType_ptr get_element_type() const;

    FASTDDS_EXPORTED_API DynamicType_ptr get_key_element_type() const;

    FASTDDS_EXPORTED_API TypeKind get_kind() const;

    FASTDDS_EXPORTED_API std::string get_name() const;

    FASTDDS_EXPORTED_API uint32_t get_total_bounds() const;

    FASTDDS_EXPORTED_API void set_kind(
            TypeKind kind);

    FASTDDS_EXPORTED_API void set_name(
            std::string name);

    FASTDDS_EXPORTED_API ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    FASTDDS_EXPORTED_API ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    FASTDDS_EXPORTED_API AnnotationDescriptor* get_annotation(
            const std::string& name) const;

    // Annotations application
    FASTDDS_EXPORTED_API bool annotation_is_extensibility() const;

    FASTDDS_EXPORTED_API bool annotation_is_mutable() const;

    FASTDDS_EXPORTED_API bool annotation_is_final() const;

    FASTDDS_EXPORTED_API bool annotation_is_appendable() const;

    FASTDDS_EXPORTED_API bool annotation_is_nested() const;

    FASTDDS_EXPORTED_API bool annotation_is_bit_bound() const;

    FASTDDS_EXPORTED_API bool annotation_is_key() const;

    FASTDDS_EXPORTED_API bool annotation_is_non_serialized() const;

    // Annotation getters
    FASTDDS_EXPORTED_API std::string annotation_get_extensibility() const;

    FASTDDS_EXPORTED_API bool annotation_get_nested() const;

    FASTDDS_EXPORTED_API uint16_t annotation_get_bit_bound() const;

    FASTDDS_EXPORTED_API bool annotation_get_key() const;

    // Annotation setters
    FASTDDS_EXPORTED_API void annotation_set_extensibility(
            const std::string& extensibility);

    FASTDDS_EXPORTED_API void annotation_set_mutable();

    FASTDDS_EXPORTED_API void annotation_set_final();

    FASTDDS_EXPORTED_API void annotation_set_appendable();

    FASTDDS_EXPORTED_API void annotation_set_nested(
            bool nested);

    FASTDDS_EXPORTED_API void annotation_set_bit_bound(
            uint16_t bit_bound);

    FASTDDS_EXPORTED_API void annotation_set_key(
            bool key);

    FASTDDS_EXPORTED_API void annotation_set_non_serialized(
            bool non_serialized);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
