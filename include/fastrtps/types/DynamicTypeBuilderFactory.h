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

#include <fastrtps/types/AnnotationParameterValue.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/custom_allocators.hpp>

#include <cassert>
#include <memory>
#include <mutex>
#include <set>

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicTypeBuilderFactory;
class DynamicTypeBuilder;
class DynamicType;

/**
 * Interface use to track dynamic objects lifetime
 * /remarks
 * This interface is only enabled if *ENABLE_DYNAMIC_MEMORY_CHECK* is defined.
 */
struct dynamic_tracker_interface
{
    //! clear collection contents
    virtual void reset() noexcept {}
    //! check if there are leakages
    virtual bool is_empty() noexcept { return true; }
    //! add primitive builder
    virtual void add_primitive(const DynamicTypeBuilder*) noexcept { return true; }
    //! add new builder
    virtual bool add(const DynamicTypeBuilder*) noexcept { return true; }
    //! remove builder
    virtual bool remove(const DynamicTypeBuilder*) noexcept { return true; }
    //! add new type
    virtual bool add(const DynamicType*) noexcept { return true; }
    //! remove type
    virtual bool remove(const DynamicType*) noexcept { return true; }
};

/**
 * @brief This class tracks dynamic type objects in order to prevent memory leakages
 */
class dtypes_memory_check
    : public dynamic_tracker_interface
{
#ifdef ENABLE_DYNAMIC_MEMORY_CHECK
    std::set<const DynamicTypeBuilder*> primitive_builders_list_; /*!< Collection of static builder instances */
    std::set<const DynamicTypeBuilder*> builders_list_; /*!< Collection of active DynamicTypeBuilder instances */
    std::set<const DynamicType*> types_list_; /*!< Collection of active DynamicType instances */
    std::mutex mutex_; /*!< atomic access to the collections */

    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;

    void reset() noexcept override;
    bool is_empty() noexcept override;
    void add_primitive(const DynamicTypeBuilder*) noexcept override;
    bool add(const DynamicTypeBuilder*) noexcept override;
    bool remove(const DynamicTypeBuilder*) noexcept override;
    bool add(const DynamicType*) noexcept override;
    bool remove(const DynamicType*) noexcept override;
#endif
};

extern dtypes_memory_check dynamic_tracker;

class TypeDescriptor;
class TypeIdentifier;
class MemberDescriptor;
class TypeObject;
class AnnotationParameterValue;

class DynamicTypeBuilderFactory
{
    using builder_allocator = detail::BuilderAllocator<DynamicTypeBuilder, DynamicTypeBuilderFactory, true>;

    // BuilderAllocator ancillary
    builder_allocator& get_allocator()
    {
        // stateful, this factory must outlive all builders
        static builder_allocator alloc{*this};
        return alloc;
    }

    friend builder_allocator;

    //! allocator callback
    void after_construction(DynamicTypeBuilder* b);

    //! allocator callback
    void before_destruction(DynamicTypeBuilder* b);

    // free any allocated resources
    void reset();

    DynamicTypeBuilderFactory() = default;

    //! auxiliary method that atomically modifies the dynamic_tracker
    DynamicTypeBuilder_ptr create_primitive_builder(TypeDescriptor&& td) noexcept;

    // TODO: doxygen
    DynamicType_ptr create_type(
            const TypeDescriptor& descriptor,
            const std::string& name = "") noexcept;

    // For DynamicTypeBuilder class only
    DynamicType_ptr create_type(
            const DynamicTypeBuilder* other);

    void build_alias_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_string8_type_code(
            const TypeDescriptor& descriptor) const;

    void build_string16_type_code(
            const TypeDescriptor& descriptor) const;

    void build_sequence_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_array_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_map_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_enum_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void build_struct_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void build_union_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void build_bitset_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void build_bitmask_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void build_annotation_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true) const;

    void set_annotation_default_value(
            AnnotationParameterValue& apv,
            const MemberDescriptor& member) const;

    void apply_type_annotations(
            AppliedAnnotationSeq& annotations,
            const TypeDescriptor& descriptor) const;
public:

    // TODO: doxygen
    RTPS_DllAPI static DynamicTypeBuilderFactory* get_instance() noexcept;

    // TODO: doxygen
    RTPS_DllAPI static ReturnCode_t delete_instance() noexcept;

    ~DynamicTypeBuilderFactory();

    // TODO: doxygen
    RTPS_DllAPI DynamicType_ptr get_primitive_type(
            TypeKind kind) noexcept;

    RTPS_DllAPI ReturnCode_t delete_builder(
            DynamicTypeBuilder* builder) noexcept;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t delete_type(
            DynamicType* type) noexcept;

    RTPS_DllAPI DynamicTypeBuilder_ptr create_custom_builder(
            const TypeDescriptor& descriptor,
            const std::string& name = "");

    RTPS_DllAPI DynamicTypeBuilder_ptr create_builder_copy(
            const DynamicTypeBuilder& type);

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int32_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint32_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int16_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint16_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int64_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint64_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float32_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float64_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float128_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_char8_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_char16_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_bool_builder();

    RTPS_DllAPI DynamicTypeBuilder_cptr& create_byte_builder();

    RTPS_DllAPI DynamicTypeBuilder_ptr create_string_builder(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_wstring_builder(
            uint32_t bound = MAX_STRING_LENGTH);

    RTPS_DllAPI DynamicTypeBuilder_ptr create_sequence_builder(
            const DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    // TODO: doxygen
    RTPS_DllAPI DynamicTypeBuilder_ptr create_sequence_builder(
            const DynamicType_ptr type,
            uint32_t bound = MAX_ELEMENTS_COUNT) noexcept;

    RTPS_DllAPI DynamicTypeBuilder_ptr create_array_builder(
            const DynamicTypeBuilder* element_type,
            const std::vector<uint32_t>& bounds);

    // TODO: doxygen
    RTPS_DllAPI DynamicTypeBuilder_ptr create_array_builder(
            const DynamicType_ptr type,
            const std::vector<uint32_t>& bounds) noexcept;

    RTPS_DllAPI DynamicTypeBuilder_ptr create_map_builder(
            DynamicTypeBuilder* key_element_type,
            DynamicTypeBuilder* element_type,
            uint32_t bound = MAX_ELEMENTS_COUNT);

    // TODO: doxygen
    RTPS_DllAPI DynamicTypeBuilder_ptr create_map_builder(
            DynamicType_ptr key_type,
            DynamicType_ptr value_type,
            uint32_t bound = MAX_ELEMENTS_COUNT) noexcept;

    // TODO: doxygen
    RTPS_DllAPI DynamicTypeBuilder_ptr create_bitmask_builder(
            uint32_t bound) noexcept;

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

    // TODO: doxygen
    RTPS_DllAPI DynamicType_ptr create_string_type(
            uint32_t bound = MAX_STRING_LENGTH) noexcept;

    // TODO: doxygen
    RTPS_DllAPI DynamicType_ptr create_wstring_type(
            uint32_t bound = MAX_STRING_LENGTH) noexcept;

    RTPS_DllAPI DynamicType_ptr create_bitset_type(
            uint32_t bound);

    RTPS_DllAPI void build_type_identifier(
            const DynamicType_ptr type,
            TypeIdentifier& identifier,
            bool complete = true) const;

    RTPS_DllAPI void build_type_identifier(
            const TypeDescriptor& descriptor,
            TypeIdentifier& identifier,
            bool complete = true) const;

    RTPS_DllAPI void build_type_object(
            const DynamicType_ptr type,
            TypeObject& object,
            bool complete = true,
            bool force = false) const;

    RTPS_DllAPI void build_type_object(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            const std::vector<MemberDescriptor>& members,
            bool complete = true,
            bool force = false) const;

    RTPS_DllAPI bool is_empty() const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
