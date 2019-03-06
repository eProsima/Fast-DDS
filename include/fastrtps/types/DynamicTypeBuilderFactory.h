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

#ifndef TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
#define TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <mutex>

//#define DISABLE_DYNAMIC_MEMORY_CHECK

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class DynamicTypeBuilder;
class TypeDescriptor;
class TypeIdentifier;
class MemberDescriptor;
class TypeObject;
class DynamicType;
class DynamicType_ptr;
class AnnotationParameterValue;

class DynamicTypeBuilderFactory
{
protected:
    DynamicTypeBuilderFactory();

    inline void add_builder_to_list(DynamicTypeBuilder* pBuilder);

    DynamicType_ptr build_type(DynamicType_ptr other);

    void build_alias_type_code(const TypeDescriptor* descriptor, TypeObject& object, bool complete = true) const;

    void build_enum_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void build_struct_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void build_union_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void build_bitset_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void build_bitmask_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void build_annotation_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*> members,
            bool complete = true) const;

    void set_annotation_default_value(
            AnnotationParameterValue& apv,
            const MemberDescriptor* member) const;

#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::vector<DynamicTypeBuilder*> builders_list_;
    mutable std::recursive_mutex mutex_;
#endif

public:
    RTPS_DllAPI static DynamicTypeBuilderFactory* get_instance();

    RTPS_DllAPI static ResponseCode delete_instance();

    ~DynamicTypeBuilderFactory();

    RTPS_DllAPI DynamicType_ptr get_primitive_type(TypeKind kind);

    RTPS_DllAPI ResponseCode delete_builder(DynamicTypeBuilder* builder);

    RTPS_DllAPI ResponseCode delete_type(DynamicType* type);

    RTPS_DllAPI DynamicTypeBuilder* create_custom_builder(
            const TypeDescriptor* descriptor,
            const std::string& name = "");

    RTPS_DllAPI DynamicTypeBuilder* create_builder_copy(const DynamicTypeBuilder* type);

    RTPS_DllAPI DynamicTypeBuilder* create_int32_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_uint32_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_int16_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_uint16_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_int64_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_uint64_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_float32_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_float64_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_float128_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_char8_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_char16_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_bool_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_byte_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_string_builder(uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder* create_wstring_builder(uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder* create_sequence_builder(
            const DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder* create_sequence_builder(
            const DynamicType_ptr type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder* create_array_builder(
            const DynamicTypeBuilder* element_type,
            const std::vector<uint32_t>& bounds);

    RTPS_DllAPI DynamicTypeBuilder* create_array_builder(
            const DynamicType_ptr type,
            const std::vector<uint32_t>& bounds);

    RTPS_DllAPI DynamicTypeBuilder* create_map_builder(
            DynamicTypeBuilder* key_element_type,
            DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder* create_map_builder(
            DynamicType_ptr key_type,
            DynamicType_ptr value_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder* create_bitmask_builder(uint32_t bound);

    RTPS_DllAPI DynamicTypeBuilder* create_bitset_builder(uint32_t bound);

    RTPS_DllAPI DynamicTypeBuilder* create_alias_builder(
            DynamicTypeBuilder* base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicTypeBuilder* create_alias_builder(
            DynamicType_ptr base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicTypeBuilder* create_enum_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_struct_builder();

    RTPS_DllAPI DynamicTypeBuilder* create_child_struct_builder(DynamicTypeBuilder* parent_type);

    RTPS_DllAPI DynamicTypeBuilder* create_union_builder(DynamicTypeBuilder* discriminator_type);

    RTPS_DllAPI DynamicTypeBuilder* create_union_builder(DynamicType_ptr discriminator_type);

    RTPS_DllAPI DynamicType_ptr create_annotation_primitive();

    RTPS_DllAPI DynamicType_ptr create_type(
            const TypeDescriptor* descriptor,
            const std::string& name = "");

    RTPS_DllAPI DynamicType_ptr create_type(const DynamicTypeBuilder* other);

    RTPS_DllAPI DynamicType_ptr create_alias_type(
            DynamicTypeBuilder* base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicType_ptr create_alias_type(
            DynamicType_ptr base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicType_ptr create_int32_type();

    RTPS_DllAPI DynamicType_ptr create_uint32_type();

    RTPS_DllAPI DynamicType_ptr create_int16_type();

    RTPS_DllAPI DynamicType_ptr create_uint16_type();

    RTPS_DllAPI DynamicType_ptr create_int64_type();

    RTPS_DllAPI DynamicType_ptr create_uint64_type();

    RTPS_DllAPI DynamicType_ptr create_float32_type();

    RTPS_DllAPI DynamicType_ptr create_float64_type();

    RTPS_DllAPI DynamicType_ptr create_float128_type();

    RTPS_DllAPI DynamicType_ptr create_char8_type();

    RTPS_DllAPI DynamicType_ptr create_char16_type();

    RTPS_DllAPI DynamicType_ptr create_bool_type();

    RTPS_DllAPI DynamicType_ptr create_byte_type();

    RTPS_DllAPI DynamicType_ptr create_string_type(uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicType_ptr create_wstring_type(uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicType_ptr create_bitset_type(uint32_t bound);

    RTPS_DllAPI void build_type_identifier(
            const DynamicType_ptr type,
            TypeIdentifier& identifier,
            bool complete = true) const;

    RTPS_DllAPI void build_type_identifier(
            const TypeDescriptor* descriptor,
            TypeIdentifier& identifier,
            bool complete = true) const;

    RTPS_DllAPI void build_type_object(
            const DynamicType_ptr type,
            TypeObject& object,
            bool complete = true) const;

    RTPS_DllAPI void build_type_object(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*>* members = nullptr,
            bool complete = true) const;

    RTPS_DllAPI bool is_empty() const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
