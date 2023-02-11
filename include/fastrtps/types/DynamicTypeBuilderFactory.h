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
#include <fastrtps/types/AnnotationParameterValue.h>

#include <cassert>
#include <deque>
#include <memory>
#include <mutex>
#include <type_traits>

//#define DISABLE_DYNAMIC_MEMORY_CHECK

namespace eprosima {
namespace fastrtps {
namespace types {
namespace detail {

template<class T, class B>
class BuilderAllocator
    : public std::allocator<T>
{
public:

    template <class Other>
    struct rebind
    {
        using other = typename std::conditional<
            std::is_same<Other, T>::value,
            BuilderAllocator,
            std::allocator<Other>>::type;
    };

    void on_deallocation(
            T* p)
    {
        // delegate into the derived class
        static_cast<B*>(this)->on_deallocation(p);
    }

};

} // namespace detail
} // namespace types
} // namespace fastrtps
} // namespace eprosima

// The allocator_traits must be specialized in the outer namespace (see N3730)
template<class T, class B>
struct std::allocator_traits<eprosima::fastrtps::types::detail::BuilderAllocator<T, B>>
{
    using BA = eprosima::fastrtps::types::detail::BuilderAllocator<T, B>;

    template <class Other>
    using rebind_alloc = typename BA::template rebind<Other>::other;

    template <class ... Types>
    static constexpr void construct(
            BA& alloc,
            T* const p,
            Types&&... args)
    {
        return std::allocator_traits<std::allocator<T>>::construct(
            alloc,
            p,
            std::forward<Types>(args)...);
    }

    static constexpr void destroy(
            BA& alloc,
            T* p)
    {
        alloc.on_deallocation(p);
        std::allocator_traits<std::allocator<T>>::destroy(alloc, p);
    }

};

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
class AnnotationParameterValue;

class DynamicTypeBuilderFactory
    : public detail::BuilderAllocator<DynamicTypeBuilder, DynamicTypeBuilderFactory>
    , public detail::BuilderAllocator<DynamicType, DynamicTypeBuilderFactory>
{
    // BuilderAllocator ancillary
    template<class T>
    const detail::BuilderAllocator<T, DynamicTypeBuilderFactory>& get_allocator() const
    {
        return static_cast<const detail::BuilderAllocator<T, DynamicTypeBuilderFactory>&>(*this);
    }

    friend class detail::BuilderAllocator<DynamicType, DynamicTypeBuilderFactory>;
    friend class detail::BuilderAllocator<DynamicTypeBuilder, DynamicTypeBuilderFactory>;

    void on_deallocation(
            DynamicType* p)
    {
        delete_type(p);
    }

    void on_deallocation(
            DynamicTypeBuilder* b)
    {
        delete_builder(b);
    }

    // free any allocated resources
    void reset();

    DynamicTypeBuilderFactory() = default;

    DynamicType_ptr create_type(
            const TypeDescriptor* descriptor,
            const std::string& name = "");

    // For DynamicTypeBuilder class only
    DynamicType_ptr create_type(
            const DynamicTypeBuilder* other);

    friend class DynamicTypeBuilder;

    inline void add_builder_to_list(
            DynamicTypeBuilder* pBuilder);

    void build_alias_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_string8_type_code(
            const TypeDescriptor* descriptor) const;

    void build_string16_type_code(
            const TypeDescriptor* descriptor) const;

    void build_sequence_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_array_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_map_type_code(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            bool complete = true) const;

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

    void apply_type_annotations(
            AppliedAnnotationSeq& annotations,
            const TypeDescriptor* descriptor) const;

#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::deque<DynamicTypeBuilder*> builders_list_;
    mutable std::mutex mutex_;
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK

public:

    RTPS_DllAPI static DynamicTypeBuilderFactory* get_instance();

    RTPS_DllAPI static ReturnCode_t delete_instance();

    ~DynamicTypeBuilderFactory();

    RTPS_DllAPI DynamicType_ptr get_primitive_type(
            TypeKind kind);

    RTPS_DllAPI ReturnCode_t delete_builder(
            DynamicTypeBuilder* builder);

    RTPS_DllAPI ReturnCode_t delete_type(
            DynamicType* type);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_custom_builder(
            const TypeDescriptor* descriptor,
            const std::string& name = "");

    RTPS_DllAPI DynamicTypeBuilder_ptr create_builder_copy(
            const DynamicTypeBuilder* type);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_int32_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_uint32_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_int16_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_uint16_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_int64_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_uint64_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_float32_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_float64_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_float128_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_char8_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_char16_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_bool_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_byte_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_string_builder(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_wstring_builder(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_sequence_builder(
            const DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_sequence_builder(
            const DynamicType_ptr type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_array_builder(
            const DynamicTypeBuilder* element_type,
            const std::vector<uint32_t>& bounds);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_array_builder(
            const DynamicType_ptr type,
            const std::vector<uint32_t>& bounds);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_map_builder(
            DynamicTypeBuilder* key_element_type,
            DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_map_builder(
            DynamicType_ptr key_type,
            DynamicType_ptr value_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_bitmask_builder(
            uint32_t bound);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_bitset_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_alias_builder(
            DynamicTypeBuilder* base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_alias_builder(
            DynamicType_ptr base_type,
            const std::string& sName);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_enum_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_struct_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_child_struct_builder(
            DynamicTypeBuilder* parent_type);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_union_builder(
            DynamicTypeBuilder* discriminator_type);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_union_builder(
            DynamicType_ptr discriminator_type);

    RTPS_DllAPI DynamicType_ptr create_annotation_primitive(
            const std::string& name);

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

    RTPS_DllAPI DynamicType_ptr create_string_type(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicType_ptr create_wstring_type(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicType_ptr create_bitset_type(
            uint32_t bound);

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
            bool complete = true,
            bool force = false) const;

    RTPS_DllAPI void build_type_object(
            const TypeDescriptor* descriptor,
            TypeObject& object,
            const std::vector<const MemberDescriptor*>* members = nullptr,
            bool complete = true,
            bool force = false) const;

    RTPS_DllAPI bool is_empty() const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
